/*
 * si5351.c
 *
 * Created on: 26 lip 2022
 *     Author: Krzysztof Markiewicz <obbo.pl>
 *
 * MIT License
 *
 * Copyright (c) 2022 Krzysztof Markiewicz
 */

#include "si5351.h"
#include <math.h>


// function prototype
si5351_err_t si5351_set_default();
si5351_err_t si5351_get_ram();
si5351_err_t si5351_set_crystal_frequency(si5351_crystal_freq_t frequency);
si5351_err_t si5351_get_revision_id(si5351_variant_t, si5351_revision_t* rev_id);
uint32_t si5351_get_pll_source_frequency(si5351_pll_reg_t pll);
si5351_err_t si5351_read_reg(uint8_t reg, uint8_t* data);
si5351_err_t si5351_write_reg(uint8_t reg, uint8_t data);
bool si5351_is_clk_source_valid(si5351_ms_clk_reg_t clk, si5351_clk_source_t* clk_source);
bool si5351_is_variant_b(si5351_variant_t variant);
bool si5351_is_variant_c(si5351_variant_t variant);
bool si5351_is_even_integer(uint16_t val);


const uint8_t si5351_clk_register[SI5351_MS_CLK_COUNT] = {
    SI5351_CLK0_CONTROL, SI5351_CLK1_CONTROL,
    SI5351_CLK2_CONTROL, SI5351_CLK3_CONTROL,
    SI5351_CLK4_CONTROL, SI5351_CLK5_CONTROL,
    SI5351_CLK6_CONTROL, SI5351_CLK7_CONTROL
};

const uint8_t si5351_multisynth_register[SI5351_MS_CLK_COUNT] = {
    SI5351_MULTISYNTH0_PARAMETERS, SI5351_MULTISYNTH1_PARAMETERS,
    SI5351_MULTISYNTH2_PARAMETERS, SI5351_MULTISYNTH3_PARAMETERS,
    SI5351_MULTISYNTH4_PARAMETERS, SI5351_MULTISYNTH5_PARAMETERS,
    SI5351_MULTISYNTH6_PARAMETERS, SI5351_MULTISYNTH7_PARAMETERS
};

const uint8_t si5351_pll_int_register[SI5351_PLL_COUNT] = {
    SI5351_CLK6_CONTROL, SI5351_CLK7_CONTROL
};

#define SI5351_GOTO_ON_ERROR(x,jump) do {       \
        result = x;                             \
        if (result != ESP_OK) {                 \
            goto jump;                          \
        }                                       \
    } while(0)

#define SI5351_DIVIDE_ROUND(n, d)       (((n) + (d) / 2) / (d))

si5351_t chip = {
        .initialised = false,
        .pll[SI5351_PLLA].configured = false,
        .pll[SI5351_PLLB].configured = false,
        .ms[SI5351_MS_CLK0].configured = false,
        .ms[SI5351_MS_CLK1].configured = false,
        .ms[SI5351_MS_CLK2].configured = false,
        .ms[SI5351_MS_CLK3].configured = false,
        .ms[SI5351_MS_CLK4].configured = false,
        .ms[SI5351_MS_CLK5].configured = false,
        .ms[SI5351_MS_CLK6].configured = false,
        .ms[SI5351_MS_CLK7].configured = false,
};



si5351_err_t si5351_init(si5351_variant_t variant,
                         uint8_t i2c_address,
                         si5351_crystal_freq_t xtal_frequency,
                         uint32_t clkin_frequency,
                         bool unbreakable)
{
    si5351_err_t result;
    chip.variant = variant;
    SI5351_GOTO_ON_ERROR(si5351_get_revision_id(variant, &(chip.rev_id)), finish);
    chip.i2c_address = i2c_address;
    uint8_t timeout = SI5351_POWERUP_TIME_ms;
    uint8_t sys_init = 0xFF;
    while (timeout && (result == SI5351_OK)) {
        uint8_t status;
        result = si5351_i2c_read(chip.i2c_address, SI5351_DEVICE_STATUS, &status, 1);
        sys_init = status & SI5351_DEVICE_STATUS_SYS_INIT_bm;
        if (sys_init == 0x00) break;
        si5351_delay_msec(1);
        timeout--;
    }
    if (result != SI5351_OK) goto finish;
    if (sys_init != 0x00) {
        result = SI5351_ERR_TIMEOUT;
        goto finish;
    }
    SI5351_GOTO_ON_ERROR(si5351_set_crystal_frequency(xtal_frequency), finish);
    if ((clkin_frequency == 0) || ((clkin_frequency >= SI5351_CLKIN_MIN) && (clkin_frequency <= SI5351_CLKIN_MAX))) {
        chip.clkin_freq = clkin_frequency;
    } else {
        chip.clkin_freq = 0;
        result = SI5351_ERR_INVALID_ARG;
    }
    if ((chip.clkin_freq == 0) && (chip.crystal_freq == 0)) result = SI5351_ERR_INVALID_ARG;
    if (result != SI5351_OK) goto finish;
    if (unbreakable) {
        // get current configuration
        si5351_get_ram();
    } else {
        // reset to default
        SI5351_GOTO_ON_ERROR(si5351_set_powerdown(), finish);
        SI5351_GOTO_ON_ERROR(si5351_set_default(), finish);
        SI5351_GOTO_ON_ERROR(si5351_reset_pll(), finish);
    }
    chip.initialised = true;
finish:
    return result;
}

si5351_err_t si5351_set_default()
{
    si5351_err_t result;
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_INTERRUPT_STATUS_STICKY, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_INTERRUPT_STATUS_MASK, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_OEB_PIN_ENABLE_CONTROL_MASK, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_set_pll_source(SI5351_PLL_XTAL, SI5351_PLL_XTAL, SI5351_CLKIN_DIVIDER1), finish);
#if (SI5351_DEFAULT_CLK_POWERDOWN == 0)
    uint8_t clk_state = 0x00;
#else
    uint8_t clk_state = 0x80;
#endif
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK0_CONTROL, clk_state), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK1_CONTROL, clk_state), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK2_CONTROL, clk_state), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK3_CONTROL, clk_state), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK4_CONTROL, clk_state), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK5_CONTROL, clk_state), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK6_CONTROL, clk_state), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK7_CONTROL, clk_state), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK3_TO_0_DISABLE_STATE, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK7_TO_4_DISABLE_STATE, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK0_INITIAL_PHASE_OFFSET, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK1_INITIAL_PHASE_OFFSET, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK2_INITIAL_PHASE_OFFSET, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK3_INITIAL_PHASE_OFFSET, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK4_INITIAL_PHASE_OFFSET, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_CLK5_INITIAL_PHASE_OFFSET, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_write_reg(SI5351_SPREAD_SPECTRUM_PARAMETERS, 0x00), finish);
    SI5351_GOTO_ON_ERROR(si5351_set_crystal_load(SI5351_CRYSTAL_LOAD_10PF), finish);
    SI5351_GOTO_ON_ERROR(si5351_set_fanout(false, false, false), finish);
finish:
    return result;
}

si5351_err_t si5351_get_ram()
{
    si5351_err_t result = SI5351_OK;
    uint8_t data;
    SI5351_GOTO_ON_ERROR(si5351_read_reg(SI5351_CRYSTAL_INTERNAL_LOAD_CAP, &data), finish);
    chip.crystal_load = (si5351_crystal_load_t)(data & SI5351_CRYSTAL_INTERNAL_LOAD_CAP_XTAL_CL_bm);
finish:
    return result;
}

si5351_err_t si5351_get_status(uint8_t* status)
{
    si5351_err_t result;
    result = si5351_i2c_read(chip.i2c_address, SI5351_DEVICE_STATUS, status, 1);
    return result;
}

si5351_err_t si5351_set_crystal_frequency(si5351_crystal_freq_t frequency)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((frequency == SI5351_CRYSTAL_FREQ_25MHZ) || (frequency == SI5351_CRYSTAL_FREQ_27MHZ) || (frequency == SI5351_CRYSTAL_NONE)) {
        chip.crystal_freq = frequency * 1000000;
        result = SI5351_OK;
    } else {
        chip.crystal_freq = 0;
    }
    return result;
}

si5351_err_t si5351_set_crystal_load(si5351_crystal_load_t cap)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((cap & ~(SI5351_CRYSTAL_INTERNAL_LOAD_CAP_XTAL_CL_bm)) == 0x00) {
        uint8_t data = ((uint8_t)cap & SI5351_CRYSTAL_INTERNAL_LOAD_CAP_XTAL_CL_bm) | SI5351_CRYSTAL_INTERNAL_LOAD_CAP_RESERVED_bm;
        result = si5351_i2c_write(chip.i2c_address, SI5351_CRYSTAL_INTERNAL_LOAD_CAP, &data, 1);
        if (result == SI5351_OK) chip.crystal_load = cap;
    }
    return result;
}

si5351_err_t si5351_set_pll_source(si5351_pll_source_t plla, si5351_pll_source_t pllb, si5351_clkin_divider_t div)
{
    si5351_err_t result;
    if ((div & ~(SI5351_PLL_INPUT_SOURCE_CLKIN_DIV_bm)) != 0) {
        result = SI5351_ERR_INVALID_ARG;
        goto finish;
    }
    if (((plla == SI5351_PLL_XTAL) || (plla == SI5351_PLL_CLKINT)) && ((pllb == SI5351_PLL_XTAL) || (pllb == SI5351_PLL_CLKINT))) {
        if (!(si5351_is_variant_c(chip.variant)) && ((plla == SI5351_PLL_CLKINT) || (pllb == SI5351_PLL_CLKINT))) {
            result = SI5351_ERR_INVALID_ARG;
        } else {
            uint8_t data = div & SI5351_PLL_INPUT_SOURCE_CLKIN_DIV_bm;
            if (plla == SI5351_PLL_CLKINT) data |= SI5351_PLL_INPUT_SOURCE_PLLA_SRC_bm;
            if (pllb == SI5351_PLL_CLKINT) data |= SI5351_PLL_INPUT_SOURCE_PLLB_SRC_bm;
            result = si5351_i2c_write(chip.i2c_address, SI5351_PLL_INPUT_SOURCE, &data, 1);
            if (result == SI5351_OK) {
                chip.clkin_divider = (1 << ((div & SI5351_PLL_INPUT_SOURCE_CLKIN_DIV_bm) >> SI5351_PLL_INPUT_SOURCE_CLKIN_DIV_bp));
                chip.pll[SI5351_PLLA].source = plla;
                chip.pll[SI5351_PLLB].source = pllb;
            }
        }
    } else {
        result = SI5351_ERR_INVALID_ARG;
    }
finish:
    return result;
}

uint32_t si5351_get_pll_source_frequency(si5351_pll_reg_t pll)
{
    uint32_t result = 0;
    if ((pll >= SI5351_PLLA) && (pll < SI5351_PLL_COUNT)) {
        if (chip.pll[pll].source == SI5351_PLL_XTAL) result = chip.crystal_freq;
        if (chip.pll[pll].source == SI5351_PLL_CLKINT) result = SI5351_DIVIDE_ROUND(chip.clkin_freq, chip.clkin_divider);
    }
    return result;
}

si5351_err_t si5351_set_pll_vco(si5351_pll_reg_t pll, uint32_t frequency)
{
    si5351_err_t result = SI5351_OK;
    if ((pll < SI5351_PLLA) || (pll >= SI5351_PLL_COUNT)) result = SI5351_ERR_INVALID_ARG;
    if (!chip.initialised) result = SI5351_ERR_NOT_INITIALISED;
    if (result != SI5351_OK) goto finish;
#if (SI5351_ALLOW_OVERCLOCKING == 0)
    if (frequency < SI5351_PLL_VCO_MIN) frequency = SI5351_PLL_VCO_MIN;
    if (frequency > SI5351_PLL_VCO_MAX) frequency = SI5351_PLL_VCO_MAX;
#endif
    uint32_t in_frequency = si5351_get_pll_source_frequency(pll);
    if ((in_frequency < SI5351_PLL_CLKIN_MIN) || (in_frequency > SI5351_PLL_CLKIN_MAX)) {
        result = SI5351_ERR_INVALID_ARG;
        goto finish;
    }
    uint8_t a = (uint8_t)(frequency / in_frequency);
    if (in_frequency * a == frequency) {
        result = si5351_set_pll_vco_integer(pll, a);
    } else {
        uint32_t c = 0xFFFFF;
        if ((pll == SI5351_PLLB) && si5351_is_variant_b(chip.variant)) c = 0xF4240;
        uint32_t b = (uint32_t)SI5351_DIVIDE_ROUND((((uint64_t)frequency % in_frequency) * c), in_frequency);
        result = si5351_set_pll_vco_fractional(pll, a, b, c);
    }
finish:
    return result;
}

si5351_err_t si5351_set_pll_vco_integer(si5351_pll_reg_t pll, uint8_t a)
{
    return si5351_set_pll_vco_fractional(pll, a, 0, 1);
}

si5351_err_t si5351_set_pll_vco_fractional(si5351_pll_reg_t pll, uint8_t a, uint32_t b, uint32_t c)
{
    si5351_err_t result = SI5351_OK;
    if ((pll < SI5351_PLLA) || (pll >= SI5351_PLL_COUNT)) result = SI5351_ERR_INVALID_ARG;
    if (!chip.initialised) result = SI5351_ERR_NOT_INITIALISED;
    if ((c == 0) || (b >= c)) result = SI5351_ERR_INVALID_ARG;
    if ((a < SI5351_PLL_INT_MIN) || (a > SI5351_PLL_INT_MAX)) result = SI5351_ERR_INVALID_ARG;
    if ((pll == SI5351_PLLB) && si5351_is_variant_b(chip.variant) && (c != 0xF4240)) result = SI5351_ERR_INVALID_ARG;
    if (result != SI5351_OK) goto finish;
    uint32_t in_frequency = si5351_get_pll_source_frequency(pll);
    if ((in_frequency < SI5351_PLL_CLKIN_MIN) || (in_frequency > SI5351_PLL_CLKIN_MAX)) {
        result = SI5351_ERR_INVALID_ARG;
        goto finish;
    }
    uint32_t frequency = (uint32_t)(SI5351_DIVIDE_ROUND((uint64_t)in_frequency * b, c) + in_frequency * a);
#if (SI5351_ALLOW_OVERCLOCKING == 0)
    if ((frequency < SI5351_PLL_VCO_MIN) || (frequency > SI5351_PLL_VCO_MAX)) {
        result = SI5351_ERR_INVALID_ARG;
        goto finish;
    }
#endif
    uint32_t p3 = c;
    if (p3 & ~((uint32_t)SI5351_MULTISYNTH_P3_bm)) {
        result = SI5351_ERR_INVALID_ARG;
        goto finish;
    }
    uint32_t p1 = (uint32_t)128 * a + ((128 * b) / c) - 512;
    if (p1 & ~((uint32_t)SI5351_MULTISYNTH_P1_bm)) {
        result = SI5351_ERR_INVALID_ARG;
        goto finish;
    }
    uint32_t p2 = 128 * b - c * ((128 * b) / c);
    if (p2 & ~((uint32_t)SI5351_MULTISYNTH_P2_bm)) {
        result = SI5351_ERR_INVALID_ARG;
        goto finish;
    }
    uint8_t pll_reg;
    switch (pll) {
        case SI5351_PLLA:
            pll_reg = SI5351_MULTISYNTH_NA_PARAMETERS;
            break;
        case SI5351_PLLB:
            pll_reg = SI5351_MULTISYNTH_NB_PARAMETERS;
            break;
        default:
            result = SI5351_ERR_INVALID_ARG;
            goto finish;
    }
    uint8_t data[SI5351_MULTISYNTH_NX_PARAMETERS_LENGTH];
    data[0] = (uint8_t)((p3 >> 8) & 0xFF);
    data[1] = (uint8_t)(p3 & 0xFF);
    data[2] = (uint8_t)((p1 >> 16) & 0x03);
    data[3] = (uint8_t)((p1 >> 8) & 0xFF);
    data[4] = (uint8_t)(p1 & 0xFF);
    data[5] = (uint8_t)(((p3 >> 12) & 0xF0) | ((p2 >> 16) & 0x0F));
    data[6] = (uint8_t)((p2 >> 8) & 0xFF);
    data[7] = (uint8_t)(p2 & 0xFF);
    result = si5351_i2c_write(chip.i2c_address, pll_reg, data, SI5351_MULTISYNTH_NX_PARAMETERS_LENGTH);
    if (result == SI5351_OK) {
        chip.pll[pll].frequency = frequency;
        chip.pll[pll].configured = true;
    }
finish:
    return result;
}

si5351_err_t si5351_set_pll_mode_integer(si5351_pll_reg_t pll, bool integer)
{
    si5351_err_t result = SI5351_OK;
    if ((pll >= SI5351_PLLA) && (pll < SI5351_PLL_COUNT)) {
        uint8_t reg = si5351_pll_int_register[pll];
        uint8_t data;
        result = si5351_i2c_read(chip.i2c_address, reg, &data, 1);
        if (result == SI5351_OK) {
            if (integer) {
                data |= SI5351_CLK_CONTROL_FB_INT_bm;
            } else {
                data &= ~(SI5351_CLK_CONTROL_FB_INT_bm);
            }
            result = si5351_i2c_write(chip.i2c_address, reg, &data, 1);
        }
    } else {
        result = SI5351_ERR_INVALID_ARG;
    }
    return result;
}

si5351_err_t si5351_get_pll_frequency(si5351_pll_reg_t pll, uint32_t* frequency)
{
    si5351_err_t result = SI5351_OK;
    if ((pll >= SI5351_PLLA) && (pll < SI5351_PLL_COUNT)) {
        if (!chip.pll[pll].configured) {
            result = SI5351_ERR_NOT_INITIALISED;
            *frequency = 0;
        } else {
            *frequency = chip.pll[pll].frequency;
        }
    } else {
        result = SI5351_ERR_INVALID_ARG;
    }
    return result;
}

si5351_err_t si5351_reset_pll()
{
    si5351_err_t result;
    uint8_t data = SI5351_PLL_RESET_PLLA_RST_bm | SI5351_PLL_RESET_PLLB_RST_bm;
    result = si5351_i2c_write(chip.i2c_address, SI5351_PLL_RESET, &data, 1);
    return result;
}

si5351_err_t si5351_set_multisynth(si5351_ms_clk_reg_t ms, si5351_pll_reg_t pll_source, uint32_t frequency)
{
    si5351_err_t result = SI5351_OK;
    if ((ms < SI5351_MS_CLK0) || (ms >= SI5351_MS_CLK_COUNT)) result = SI5351_ERR_INVALID_ARG;
    if ((pll_source < SI5351_PLLA) || (pll_source >= SI5351_PLL_COUNT)) result = SI5351_ERR_INVALID_ARG;
    if (result != SI5351_OK) goto finish;
    if (!chip.pll[pll_source].configured) {
        result = SI5351_ERR_NOT_INITIALISED;
        goto finish;
    }
    uint32_t vco_freq = chip.pll[pll_source].frequency;
    uint16_t a = (uint16_t)(vco_freq / frequency);
    switch (ms) {
        case SI5351_MS_CLK0:
        case SI5351_MS_CLK1:
        case SI5351_MS_CLK2:
        case SI5351_MS_CLK3:
        case SI5351_MS_CLK4:
        case SI5351_MS_CLK5:
#if (SI5351_ALLOW_OVERCLOCKING == 0)
            if ((chip.rev_id == SI5351_REVISION_A) && (frequency > SI5351_REVA_MULTISYNTH_FREQUENCY_MAX)) result = SI5351_ERR_INVALID_ARG;
            if ((chip.rev_id == SI5351_REVISION_B) && (frequency > SI5351_REVB_MULTISYNTH_FREQUENCY_MAX)) result = SI5351_ERR_INVALID_ARG;
#endif
            if ((a < SI5351_MULTISYNTH_FRAC_0_TO_5_MIN) && (a >= SI5351_MULTISYNTH_INT_0_TO_5_DIV4) && si5351_is_even_integer(a)) {
                if ((vco_freq < a * (frequency + 1)) && (vco_freq > a * (frequency - 1))) {
                    result = si5351_set_multisynth_integer(ms, pll_source, a);
                } else {
                    result = SI5351_ERR_INVALID_ARG;
                }
                if (result != SI5351_OK) goto finish;
            } else {
#if (SI5351_ALLOW_OVERCLOCKING == 0)
                if (vco_freq > (frequency * SI5351_MULTISYNTH_FRAC_0_TO_5_MAX)) result = SI5351_ERR_INVALID_ARG;
                if (vco_freq < (frequency * SI5351_MULTISYNTH_FRAC_0_TO_5_MIN)) result = SI5351_ERR_INVALID_ARG;
#endif
                if (result != SI5351_OK) goto finish;
                if ((vco_freq < a * (frequency + 1)) && (vco_freq > a * (frequency - 1))) {
                    result = si5351_set_multisynth_integer(ms, pll_source, a);
                } else {
                    uint32_t c = 0xFFFFF;
                    uint32_t b = (uint32_t)SI5351_DIVIDE_ROUND((uint64_t)(vco_freq % frequency) * c, frequency);
                    result = si5351_set_multisynth_fractional(ms, pll_source, a, b, c);
                }
            }
            break;
        case SI5351_MS_CLK6:
        case SI5351_MS_CLK7:
#if (SI5351_ALLOW_OVERCLOCKING == 0)
            if (vco_freq > (frequency * SI5351_MULTISYNTH_INT_0_TO_7_MAX)) result = SI5351_ERR_INVALID_ARG;
            if (vco_freq < (frequency * SI5351_MULTISYNTH_INT_0_TO_7_MIN)) result = SI5351_ERR_INVALID_ARG;
            if (!si5351_is_even_integer(a)) result = SI5351_ERR_INVALID_ARG;
#endif
            if ((vco_freq < a * (frequency + 1)) && (vco_freq > a * (frequency - 1))) result = SI5351_ERR_INVALID_ARG;
            if (result != SI5351_OK) goto finish;
            result = si5351_set_multisynth_integer(ms, pll_source, a);
            break;
        default:
            result = SI5351_ERR_INVALID_ARG;
    }
finish:
    return result;
}

si5351_err_t si5351_set_multisynth_integer(si5351_ms_clk_reg_t ms, si5351_pll_reg_t pll_source, uint16_t a)
{
    return si5351_set_multisynth_fractional(ms, pll_source, a, 0, 1);
}

si5351_err_t si5351_set_multisynth_fractional(si5351_ms_clk_reg_t ms, si5351_pll_reg_t pll_source, uint16_t a, uint32_t b, uint32_t c)
{
    si5351_err_t result = SI5351_OK;
    if ((ms < SI5351_MS_CLK0) || (ms >= SI5351_MS_CLK_COUNT)) result = SI5351_ERR_INVALID_ARG;
    if ((pll_source < SI5351_PLLA) || (pll_source >= SI5351_PLL_COUNT)) result = SI5351_ERR_INVALID_ARG;
    if (result != SI5351_OK) goto finish;
    if (!chip.pll[pll_source].configured) result = SI5351_ERR_NOT_INITIALISED;
    if ((c == 0) || (b >= c)) result = SI5351_ERR_INVALID_ARG;
    if (result != SI5351_OK) goto finish;
    bool set_div4 = false;
    bool set_integer = false;
    switch (ms) {
        case SI5351_MS_CLK0:
        case SI5351_MS_CLK1:
        case SI5351_MS_CLK2:
        case SI5351_MS_CLK3:
        case SI5351_MS_CLK4:
        case SI5351_MS_CLK5:
            if ((a == SI5351_MULTISYNTH_FRAC_0_TO_5_MAX) && (b > 0)) result = SI5351_ERR_INVALID_ARG;
            if ((a < SI5351_MULTISYNTH_FRAC_0_TO_5_MIN) || (a > SI5351_MULTISYNTH_FRAC_0_TO_5_MAX)) {
                if (b == 0) {
                    if (a == SI5351_MULTISYNTH_INT_0_TO_5_DIV4) {
                        set_div4 = true;
                        set_integer = true;
                        c = 1;
                    } else if ((a >= SI5351_MULTISYNTH_INT_0_TO_7_MIN) && (a <= SI5351_MULTISYNTH_INT_0_TO_7_MAX) && si5351_is_even_integer(a)) {
                        set_integer = true;
                        c = 1;
                    } else {
                        result = SI5351_ERR_INVALID_ARG;
                    }
                } else {
                    result = SI5351_ERR_INVALID_ARG;
                }
            }
            break;
        case SI5351_MS_CLK6:
        case SI5351_MS_CLK7:
#if (SI5351_ALLOW_OVERCLOCKING == 0)
            if ((a < SI5351_MULTISYNTH_INT_0_TO_7_MIN) || (a > SI5351_MULTISYNTH_INT_0_TO_7_MAX)) result = SI5351_ERR_INVALID_ARG;
            if (!(si5351_is_even_integer(a))) result = SI5351_ERR_INVALID_ARG;
#endif
            b = 0;
            c = 1;
            break;
        default:
            result = SI5351_ERR_INVALID_ARG;
    }
    if (result != SI5351_OK) goto finish;
    uint8_t ms_reg = si5351_multisynth_register[ms];
    uint8_t data[SI5351_MULTISYNTH_0_TO_5_PARAMETERS_LENGTH];
    if ((ms == SI5351_MS_CLK6) || (ms == SI5351_MS_CLK7)) {
        data[0] = (uint8_t)(a & 0xFF);
        result = si5351_i2c_write(chip.i2c_address, ms_reg, data, 1);
    } else {
        uint32_t p3 = c;
        if (p3 & ~((uint32_t)SI5351_MULTISYNTH_P3_bm)) {
            result = SI5351_ERR_INVALID_ARG;
            goto finish;
        }
        uint32_t p1 = (uint32_t)128 * a + ((128 * b) / c) - 512;
        if (p1 & ~((uint32_t)SI5351_MULTISYNTH_P1_bm)) {
            result = SI5351_ERR_INVALID_ARG;
            goto finish;
        }
        uint32_t p2 = 128 * b - c * ((128 * b) / c);
        if (p2 & ~((uint32_t)SI5351_MULTISYNTH_P2_bm)) {
            result = SI5351_ERR_INVALID_ARG;
            goto finish;
        }
        data[0] = (uint8_t)((p3 >> 8) & 0xFF);
        data[1] = (uint8_t)(p3 & 0xFF);
        data[2] = (uint8_t)((p1 >> 16) & 0x03);
        if (set_div4) data[2] |= SI5351_MULTISYNTH0_PARAMETERS_MS_DIV4_bm;
        data[3] = (uint8_t)((p1 >> 8) & 0xFF);
        data[4] = (uint8_t)(p1 & 0xFF);
        data[5] = (uint8_t)(((p3 >> 12) & 0xF0) | ((p2 >> 16) & 0x0F));
        data[6] = (uint8_t)((p2 >> 8) & 0xFF);
        data[7] = (uint8_t)(p2 & 0xFF);
        result = si5351_i2c_write(chip.i2c_address, ms_reg, data, SI5351_MULTISYNTH_0_TO_5_PARAMETERS_LENGTH);
        if (set_integer && (result == SI5351_OK)) result = si5351_set_multisynth_mode_integer(ms, true);
    }
    if (result == SI5351_OK) {
        result = si5351_i2c_read(chip.i2c_address, si5351_clk_register[ms], data, 1);
        switch (pll_source) {
            case SI5351_PLLA:
                *data &= ~(SI5351_CLK_CONTROL_MS_SRC_bm);
                break;
            case SI5351_PLLB:
                *data |= SI5351_CLK_CONTROL_MS_SRC_bm;
                break;
            default:
                result = SI5351_ERR_INVALID_ARG;
                goto finish;
        }
        result = si5351_i2c_write(chip.i2c_address, si5351_clk_register[ms], data, 1);
        if (result == SI5351_OK) {
            chip.ms[ms].frequency = (uint32_t)SI5351_DIVIDE_ROUND((uint64_t)chip.pll[pll_source].frequency * c, c * a + b);
            chip.ms[ms].configured = true;
        }
    }
finish:
    return result;
}

si5351_err_t si5351_set_multisynth_mode_integer(si5351_ms_clk_reg_t ms, bool integer)
{
    si5351_err_t result;
    uint8_t data;
    switch (ms) {
        case SI5351_MS_CLK0:
        case SI5351_MS_CLK1:
        case SI5351_MS_CLK2:
        case SI5351_MS_CLK3:
        case SI5351_MS_CLK4:
        case SI5351_MS_CLK5:
            result = si5351_i2c_read(chip.i2c_address, si5351_clk_register[ms], &data, 1);
            if (result == SI5351_OK) {
                if (integer) {
                    data |= SI5351_CLK_CONTROL_MS_INT_bm;
                } else {
                    data &= ~(SI5351_CLK_CONTROL_MS_INT_bm);
                }
                result = si5351_i2c_write(chip.i2c_address, si5351_clk_register[ms], &data, 1);
            }
            break;
        default:
            result = SI5351_ERR_INVALID_ARG;
    }
    return result;
}

si5351_err_t si5351_get_multisynth_frequency(si5351_ms_clk_reg_t ms, uint32_t* frequency)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((ms >= SI5351_MS_CLK0) && (ms < SI5351_MS_CLK_COUNT)) {
        if (!chip.ms[ms].configured) {
            result = SI5351_ERR_NOT_INITIALISED;
            *frequency = 0;
        } else {
            *frequency = chip.ms[ms].frequency;
        }
    }
    return result;
}

si5351_err_t si5351_set_fanout(bool clkin, bool xo, bool ms)
{
    si5351_err_t result;
    uint8_t data = 0x00;
    if (clkin && si5351_is_variant_c(chip.variant)) data |= SI5351_FANOUT_ENABLE_CLKIN_bm;
    if (xo) data |= SI5351_FANOUT_ENABLE_XO_bm;
    if (ms) data |= SI5351_FANOUT_ENABLE_MS_bm;
    result = si5351_i2c_write(chip.i2c_address, SI5351_FANOUT_ENABLE, &data, 1);
    if (result == SI5351_OK) chip.fanout_bm = data;
    return result;
}

si5351_err_t si5351_set_clk_disable_state(si5351_ms_clk_reg_t clk, si5351_clk_state_t state)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((clk < SI5351_MS_CLK0) || (clk >= SI5351_MS_CLK_COUNT)) goto finish;
    if ((state & ~(SI5351_CLK0_TO_7_DISABLE_STATE_CLK_bm)) != 0x00) goto finish;
    uint8_t reg;
    if (clk <= SI5351_MS_CLK3) {
        reg = SI5351_CLK3_TO_0_DISABLE_STATE;
    } else {
        reg = SI5351_CLK7_TO_4_DISABLE_STATE;
        clk = clk - 4;
    }
    uint8_t data;
    result = si5351_i2c_read(chip.i2c_address, reg, &data, 1);
    if (result == SI5351_OK) {
        data &= ~(SI5351_CLK0_TO_7_DISABLE_STATE_CLK_bm << (2 * clk));
        data |= ((state & SI5351_CLK0_TO_7_DISABLE_STATE_CLK_bm) << (2 * clk));
        result = si5351_i2c_write(chip.i2c_address, reg, &data, 1);
    }
finish:
    return result;
}

si5351_err_t si5351_set_clk(si5351_ms_clk_reg_t clk,
                            bool powerup,
                            bool inverted,
                            si5351_clk_source_t clk_source,
                            si5351_clk_r_div_t r,
                            si5351_drv_strength_t drv_strength)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((clk < SI5351_MS_CLK0) || (clk >= SI5351_MS_CLK_COUNT)) goto finish;
    if ((r & ~(SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_bm)) != 0) goto finish;
    if ((drv_strength & ~(SI5351_CLK_CONTROL_CLK_IDRV_bm)) != 0) goto finish;
    if (si5351_is_clk_source_valid(clk, &clk_source)) {
        uint8_t data;
        result = si5351_i2c_read(chip.i2c_address, si5351_clk_register[clk], &data, 1);
        if (result == SI5351_OK) {
            data &= ~(SI5351_CLK_CONTROL_CLK_SRC_bm);
            data |= clk_source & SI5351_CLK_CONTROL_CLK_SRC_bm;
            data &= ~(SI5351_CLK_CONTROL_CLK_IDRV_bm);
            data |= drv_strength & SI5351_CLK_CONTROL_CLK_IDRV_bm;
            data &= ~(SI5351_CLK_CONTROL_CLK_INV_bm);
            if (inverted) data |= SI5351_CLK_CONTROL_CLK_INV_bm;
            data &= ~(SI5351_CLK_CONTROL_CLK_PDN_bm);
            if (!powerup) data |= SI5351_CLK_CONTROL_CLK_PDN_bm;
            result = si5351_i2c_write(chip.i2c_address, si5351_clk_register[clk], &data, 1);
            if (result == SI5351_OK) {
                result = si5351_set_clk_r_div(clk, r);
            }
        }
    }
finish:
    return result;
}

si5351_err_t si5351_set_clk_initial_phase(si5351_ms_clk_reg_t clk, uint8_t phase)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((clk >= SI5351_MS_CLK0) && (clk <= SI5351_MS_CLK5)) {
        if (phase > SI5351_CLK_INITIAL_PHASE_OFFSET_bm) phase = SI5351_CLK_INITIAL_PHASE_OFFSET_bm;
        result = si5351_i2c_write(chip.i2c_address, SI5351_CLK0_INITIAL_PHASE_OFFSET + (uint8_t)clk, &phase, 1);
    }
    return result;
}

si5351_err_t si5351_set_clk_inverted(si5351_ms_clk_reg_t clk, bool inverted)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((clk >= SI5351_MS_CLK0) && (clk < SI5351_MS_CLK_COUNT)) {
        uint8_t data;
        result = si5351_i2c_read(chip.i2c_address, si5351_clk_register[clk], &data, 1);
        if (result == SI5351_OK) {
            if (inverted) {
                data |= SI5351_CLK_CONTROL_CLK_INV_bm;
            } else {
                data &= ~(SI5351_CLK_CONTROL_CLK_INV_bm);
            }
            result = si5351_i2c_write(chip.i2c_address, si5351_clk_register[clk], &data, 1);
        }
    }
    return result;
}

si5351_err_t si5351_set_clk_r_div(si5351_ms_clk_reg_t clk, si5351_clk_r_div_t r)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((clk >= SI5351_MS_CLK0) && (clk < SI5351_MS_CLK_COUNT) && ((r & ~(SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_bm)) == 0)) {
        uint8_t data;
        uint8_t reg = si5351_multisynth_register[clk] + 2;
        result = si5351_i2c_read(chip.i2c_address, reg, &data, 1);
        if (result == SI5351_OK) {
            data &= ~(SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_bm);
            data |= r;
            result = si5351_i2c_write(chip.i2c_address, reg, &data, 1);
        }
    }
    return result;
}

si5351_err_t si5351_set_clk_strength(si5351_ms_clk_reg_t clk, si5351_drv_strength_t drv_strength)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((clk >= SI5351_MS_CLK0) && (clk < SI5351_MS_CLK_COUNT) && ((drv_strength & ~(SI5351_CLK_CONTROL_CLK_IDRV_bm)) == 0x00)) {
        uint8_t data;
        result = si5351_i2c_read(chip.i2c_address, si5351_clk_register[clk], &data, 1);
        if (result == SI5351_OK) {
            data &= ~(SI5351_CLK_CONTROL_CLK_IDRV_bm);
            data |= drv_strength & SI5351_CLK_CONTROL_CLK_IDRV_bm;
            result = si5351_i2c_write(chip.i2c_address, si5351_clk_register[clk], &data, 1);
        }
    }
    return result;
}

bool si5351_is_clk_source_valid(si5351_ms_clk_reg_t clk, si5351_clk_source_t* clk_source)
{
    bool result = false;
    if ((clk >= SI5351_MS_CLK0) && (clk < SI5351_MS_CLK_COUNT)) {
        switch (clk) {
            case SI5351_MS_CLK0:
                if (*clk_source == SI5351_CLK_SOURCE_MS_0_OR_4) *clk_source = SI5351_CLK_SOURCE_MS_X;
                break;
            case SI5351_MS_CLK1:
            case SI5351_MS_CLK2:
            case SI5351_MS_CLK3:
                if (chip.fanout_bm & SI5351_FANOUT_ENABLE_MS_bm) {
                    if ((*clk_source == SI5351_CLK_SOURCE_MS_0_OR_4) && chip.ms[SI5351_MS_CLK0].configured) result = true;
                }
                break;
            case SI5351_MS_CLK4:
                if (*clk_source == SI5351_CLK_SOURCE_MS_0_OR_4) *clk_source = SI5351_CLK_SOURCE_MS_X;
                break;
            case SI5351_MS_CLK5:
            case SI5351_MS_CLK6:
            case SI5351_MS_CLK7:
                if (chip.fanout_bm & SI5351_FANOUT_ENABLE_MS_bm) {
                    if ((*clk_source == SI5351_CLK_SOURCE_MS_0_OR_4) && chip.ms[SI5351_MS_CLK4].configured) result = true;
                }
                break;
            default:
                result = false;
        }
        if ((*clk_source == SI5351_CLK_SOURCE_MS_X) && chip.ms[clk].configured) result = true;
        if (*clk_source == SI5351_CLK_SOURCE_XTAL) {
            if ((chip.fanout_bm & SI5351_FANOUT_ENABLE_XO_bm) && (chip.crystal_freq > 0)) result = true;
        }
        if (*clk_source == SI5351_CLK_SOURCE_CLKIN) {
            if ((chip.fanout_bm & SI5351_FANOUT_ENABLE_CLKIN_bm) && (chip.clkin_freq > 0)) result = true;
        }
    }
    return result;
}

si5351_err_t si5351_set_clk_source(si5351_ms_clk_reg_t clk, si5351_clk_source_t clk_source)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((clk >= SI5351_MS_CLK0) && (clk < SI5351_MS_CLK_COUNT)) {
        if (si5351_is_clk_source_valid(clk, &clk_source)) {
            uint8_t data;
            result = si5351_i2c_read(chip.i2c_address, si5351_clk_register[clk], &data, 1);
            if (result == SI5351_OK) {
                data &= ~(SI5351_CLK_CONTROL_CLK_SRC_bm);
                data |= (clk_source & SI5351_CLK_CONTROL_CLK_SRC_bm);
                result = si5351_i2c_write(chip.i2c_address, si5351_clk_register[clk], &data, 1);
            }
        }
    }
    return result;
}

si5351_err_t si5351_set_clk_power_enable(si5351_ms_clk_reg_t clk, bool enable)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((clk >= SI5351_MS_CLK0) && (clk < SI5351_MS_CLK_COUNT)) {
        uint8_t data;
        result = si5351_i2c_read(chip.i2c_address, si5351_clk_register[clk], &data, 1);
        if (result == SI5351_OK) {
            if (enable) {
                data &= ~(SI5351_CLK_CONTROL_CLK_PDN_bm);
            } else {
                data |= SI5351_CLK_CONTROL_CLK_PDN_bm;
            }
            result = si5351_i2c_write(chip.i2c_address, si5351_clk_register[clk], &data, 1);
        }
    }
    return result;
}

si5351_err_t si5351_set_output_enable(si5351_ms_clk_reg_t clk, bool enable)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    if ((clk >= SI5351_MS_CLK0) && (clk < SI5351_MS_CLK_COUNT)) {
        uint8_t data;
        result = si5351_i2c_read(chip.i2c_address, SI5351_OUTPUT_ENABLE_CONTROL, &data, 1);
        if (result == SI5351_OK) {
            if (enable) {
                data &= ~(1 << clk);
            } else {
                data |= (1 << clk);
            }
            result = si5351_i2c_write(chip.i2c_address, SI5351_OUTPUT_ENABLE_CONTROL, &data, 1);
        }
    }
    return result;
}

si5351_err_t si5351_set_powerdown()
{
    si5351_err_t result = SI5351_ERR_INVALID_STATE;
    for (int i = SI5351_MS_CLK0; i < SI5351_MS_CLK_COUNT; i++) {
        result = si5351_set_output_enable(i, false);
        if (result != SI5351_OK) break;
        result = si5351_set_clk_power_enable(i, false);
        if (result != SI5351_OK) break;
    }
    return result;
}

si5351_err_t si5351_read_reg(uint8_t reg, uint8_t* data)
{
    si5351_err_t result;
    result = si5351_i2c_read(chip.i2c_address, reg, data, 1);
    return result;
}

si5351_err_t si5351_write_reg(uint8_t reg, uint8_t data)
{
    si5351_err_t result;
    result = si5351_i2c_write(chip.i2c_address, reg, &data, 1);
    return result;
}

si5351_err_t si5351_get_revision_id(si5351_variant_t variant, si5351_revision_t* rev_id)
{
    si5351_err_t result = SI5351_ERR_INVALID_ARG;
    switch (variant) {
        case SI5351_VARIANT_A_A_GM:
        case SI5351_VARIANT_A_A_GU:
        case SI5351_VARIANT_A_A_GT:
        case SI5351_VARIANT_B_A_GM:
        case SI5351_VARIANT_B_A_GU:
        case SI5351_VARIANT_C_A_GM:
        case SI5351_VARIANT_C_A_GU:
            *rev_id = SI5351_REVISION_A;
            result = SI5351_OK;
            break;
        case SI5351_VARIANT_A_B_GM:
        case SI5351_VARIANT_A_B_GM1:
        case SI5351_VARIANT_A_B_GT:
        case SI5351_VARIANT_B_B_GM:
        case SI5351_VARIANT_B_B_GM1:
        case SI5351_VARIANT_C_B_GM:
        case SI5351_VARIANT_C_B_GM1:
            *rev_id = SI5351_REVISION_B;
            result = SI5351_OK;
            break;
        default:
            *rev_id = SI5351_REVISION_A;
    }
    return result;
}

bool si5351_is_variant_b(si5351_variant_t variant)
{
    bool result;
    switch (variant) {
        case SI5351_VARIANT_B_A_GM:
        case SI5351_VARIANT_B_A_GU:
        case SI5351_VARIANT_B_B_GM:
        case SI5351_VARIANT_B_B_GM1:
            result = true;
            break;
        default:
            result = false;
    }
    return result;
}

bool si5351_is_variant_c(si5351_variant_t variant)
{
    bool result;
    switch (variant) {
        case SI5351_VARIANT_C_A_GM:
        case SI5351_VARIANT_C_A_GU:
        case SI5351_VARIANT_C_B_GM:
        case SI5351_VARIANT_C_B_GM1:
            result = true;
            break;
        default:
            result = false;
    }
    return result;
}

bool si5351_is_even_integer(uint16_t val)
{
    bool result;
    if (val % 2 == 0) {
        result = true;
    } else {
        result = false;
    }
    return result;
}

