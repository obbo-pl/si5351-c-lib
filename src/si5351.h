/*
 * si5351.h
 *
 * Created on: 26 lip 2022
 *     Author: Krzysztof Markiewicz <obbo.pl>
 *
 * MIT License
 *
 * Copyright (c) 2022 Krzysztof Markiewicz
 */

#ifndef _SI5351_H_
#define _SI5351_H_


#include "si5351_def.h"
#include <stdbool.h>
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif


#define SI5351_DEFAULT_CLK_POWERDOWN        1
#define SI5351_ALLOW_OVERCLOCKING           0

typedef enum {
    SI5351_MS_CLK0,
    SI5351_MS_CLK1,
    SI5351_MS_CLK2,
    SI5351_MS_CLK3,
    SI5351_MS_CLK4,
    SI5351_MS_CLK5,
    SI5351_MS_CLK6,
    SI5351_MS_CLK7,
    SI5351_MS_CLK_COUNT
} si5351_ms_clk_reg_t;

typedef enum {
    SI5351_REVISION_A,
    SI5351_REVISION_B,
} si5351_revision_t;

typedef enum {
    SI5351_PLLA,
    SI5351_PLLB,
    SI5351_PLL_COUNT
} si5351_pll_reg_t;

typedef enum {                  
    SI5351_CLKIN_DIVIDER1       = SI5351_PLL_INPUT_SOURCE_CLKIN_DIV1_bm,
    SI5351_CLKIN_DIVIDER2       = SI5351_PLL_INPUT_SOURCE_CLKIN_DIV2_bm,
    SI5351_CLKIN_DIVIDER4       = SI5351_PLL_INPUT_SOURCE_CLKIN_DIV4_bm,
    SI5351_CLKIN_DIVIDER8       = SI5351_PLL_INPUT_SOURCE_CLKIN_DIV8_bm
} si5351_clkin_divider_t;

typedef enum {
    SI5351_PLL_XTAL,
    SI5351_PLL_CLKINT
} si5351_pll_source_t;

typedef enum {
    SI5351_CRYSTAL_LOAD_6PF     = SI5351_CRYSTAL_INTERNAL_LOAD_CAP_XTAL_6PF_bm,
    SI5351_CRYSTAL_LOAD_8PF     = SI5351_CRYSTAL_INTERNAL_LOAD_CAP_XTAL_8PF_bm,
    SI5351_CRYSTAL_LOAD_10PF    = SI5351_CRYSTAL_INTERNAL_LOAD_CAP_XTAL_10PF_bm
} si5351_crystal_load_t;

typedef enum {
    SI5351_CRYSTAL_NONE         = 0,
    SI5351_CRYSTAL_FREQ_25MHZ   = 25,
    SI5351_CRYSTAL_FREQ_27MHZ   = 27
} si5351_crystal_freq_t;

typedef enum {
    SI5351_CLK_SOURCE_XTAL      = SI5351_CLK_CONTROL_CLK_SRC_XTAL_bm,
    SI5351_CLK_SOURCE_CLKIN     = SI5351_CLK_CONTROL_CLK_SRC_CLKIN_bm,
    SI5351_CLK_SOURCE_MS_0_OR_4 = SI5351_CLK_CONTROL_CLK_SRC_MULTISYNTH_0_OR_4_bm,
    SI5351_CLK_SOURCE_MS_X      = SI5351_CLK_CONTROL_CLK_SRC_MULTISYNTH_X_bm
} si5351_clk_source_t;

typedef enum {
    SI5351_CLK_STATE_LOW        = SI5351_CLK0_TO_7_DISABLE_STATE_CLK_LOW_bm,
    SI5351_CLK_STATE_HIGH       = SI5351_CLK0_TO_7_DISABLE_STATE_CLK_HIGH_bm,
    SI5351_CLK_STATE_HIGHIMP    = SI5351_CLK0_TO_7_DISABLE_STATE_CLK_HIGHIMP_bm,
    SI5351_CLK_STATE_NEVER      = SI5351_CLK0_TO_7_DISABLE_STATE_CLK_NEVER_bm
} si5351_clk_state_t;

typedef enum {
    SI5351_DRIVE_STRENGTH_2mA   = SI5351_CLK_CONTROL_CLK_IDRV_2mA_bm,
    SI5351_DRIVE_STRENGTH_4mA   = SI5351_CLK_CONTROL_CLK_IDRV_4mA_bm,
    SI5351_DRIVE_STRENGTH_6mA   = SI5351_CLK_CONTROL_CLK_IDRV_6mA_bm,
    SI5351_DRIVE_STRENGTH_8mA   = SI5351_CLK_CONTROL_CLK_IDRV_8mA_bm
} si5351_drv_strength_t;

typedef enum {
    SI5351_CLK_R_DIVIDER_1      = SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_1_bm,
    SI5351_CLK_R_DIVIDER_2      = SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_2_bm,
    SI5351_CLK_R_DIVIDER_4      = SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_4_bm,
    SI5351_CLK_R_DIVIDER_8      = SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_8_bm,
    SI5351_CLK_R_DIVIDER_16     = SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_16_bm,
    SI5351_CLK_R_DIVIDER_32     = SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_32_bm,
    SI5351_CLK_R_DIVIDER_64     = SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_64_bm,
    SI5351_CLK_R_DIVIDER_128    = SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_128_bm,
} si5351_clk_r_div_t;


typedef struct {
    bool configured;
    si5351_pll_source_t source;
    uint32_t frequency;
} si5351_pll_t;

typedef struct {
    bool configured;
    uint32_t frequency;
} si5351_ms_t;

typedef enum si5351_variant si5351_variant_t;

typedef struct {
    bool initialised;
    si5351_variant_t variant;
    si5351_revision_t rev_id;
    uint8_t i2c_address;
    uint32_t crystal_freq;
    si5351_crystal_load_t crystal_load;
    uint32_t clkin_freq;
    uint8_t clkin_divider;
    si5351_pll_t pll[SI5351_PLL_COUNT];
    si5351_ms_t ms[SI5351_MS_CLK_COUNT];
} si5351_t;



#if ARDUINO >= 100
#include <Arduino.h>
#include "i2c_master.h"

typedef uint8_t si5351_err_t;

#define SI5351_OK                       0x00
#define SI5351_ERR_TIMEOUT              0x11
#define SI5351_ERR_INVALID_STATE        0x12
#define SI5351_ERR_NOT_INITIALISED      0x13
#define SI5351_ERR_INVALID_ARG          0x14


#define si5351_delay_msec(x)        do {            \
        delay(x);                                   \
    } while(0)

inline si5351_err_t si5351_i2c_read(uint8_t i2c_addr, uint8_t reg, uint8_t *data, uint8_t count)
{
    i2c_addr &= 0x7F;
    return (si5351_err_t)i2c_master_read_reg(i2c_addr, reg, data, count);
}

inline si5351_err_t si5351_i2c_write(uint8_t i2c_addr, uint8_t reg, uint8_t *data, uint8_t count)
{
    i2c_addr &= 0x7F;
    return (si5351_err_t)i2c_master_write_reg(i2c_addr, reg, data, count);
}

#elif defined(ESP_PLATFORM)
#include "i2c_master.h"
#include "esp_err.h"

typedef esp_err_t si5351_err_t;

#define SI5351_OK                       ESP_OK
#define SI5351_ERR_TIMEOUT              ESP_ERR_TIMEOUT
#define SI5351_ERR_INVALID_STATE        ESP_ERR_INVALID_STATE
#define SI5351_ERR_NOT_INITIALISED      ESP_ERR_INVALID_STATE
#define SI5351_ERR_INVALID_ARG          ESP_ERR_INVALID_ARG


#define si5351_delay_msec(x)        do {            \
        vTaskDelay((x) / portTICK_PERIOD_MS);       \
    } while(0)

inline si5351_err_t si5351_i2c_read(uint8_t i2c_addr, uint8_t reg, uint8_t *data, uint8_t count)
{
    i2c_addr &= 0x7F;
    return (si5351_err_t)i2c_master_read_reg(i2c_addr, reg, data, count);
}

inline si5351_err_t si5351_i2c_write(uint8_t i2c_addr, uint8_t reg, uint8_t *data, uint8_t count)
{
    i2c_addr &= 0x7F;
    return (si5351_err_t)i2c_master_write_reg(i2c_addr, reg, data, count);
}

#else
// Here you can put functions specific to your framework
#endif


si5351_err_t si5351_init(si5351_variant_t variant,
                         uint8_t i2c_address,
                         si5351_crystal_freq_t xtal_frequency,
                         uint32_t clkin_frequency,
                         bool unbreakable);
si5351_err_t si5351_get_status(uint8_t* status);
si5351_err_t si5351_set_crystal_load(si5351_crystal_load_t cap);
si5351_err_t si5351_set_pll_source(si5351_pll_source_t plla, si5351_pll_source_t pllb, si5351_clkin_divider_t divider);
si5351_err_t si5351_set_pll_vco(si5351_pll_reg_t pll, uint32_t frequency);
si5351_err_t si5351_set_pll_vco_integer(si5351_pll_reg_t pll, uint8_t a);
si5351_err_t si5351_set_pll_vco_fractional(si5351_pll_reg_t pll, uint8_t a, uint32_t b, uint32_t c);
si5351_err_t si5351_set_pll_mode_integer(si5351_pll_reg_t pll, bool integer);
si5351_err_t si5351_get_pll_frequency(si5351_pll_reg_t pll, uint32_t* frequency);
si5351_err_t si5351_set_multisynth(si5351_ms_clk_reg_t ms, si5351_pll_reg_t pll_source, uint32_t frequency);
si5351_err_t si5351_set_multisynth_integer(si5351_ms_clk_reg_t ms, si5351_pll_reg_t pll_source, uint16_t a);
si5351_err_t si5351_set_multisynth_fractional(si5351_ms_clk_reg_t ms, si5351_pll_reg_t pll_source, uint16_t a, uint32_t b, uint32_t c);
si5351_err_t si5351_set_multisynth_mode_integer(si5351_ms_clk_reg_t ms, bool integer);
si5351_err_t si5351_get_multisynth_frequency(si5351_ms_clk_reg_t ms, uint32_t* frequency);
si5351_err_t si5351_set_fanout(bool clkin, bool xtal, bool ms);
si5351_err_t si5351_set_clk_disable_state(si5351_ms_clk_reg_t clk, si5351_clk_state_t state);
si5351_err_t si5351_set_clk(si5351_ms_clk_reg_t clk,
                            bool powerup,
                            bool inverted,
                            si5351_clk_source_t clk_source,
                            si5351_clk_r_div_t r,
                            si5351_drv_strength_t drv_strength);
si5351_err_t si5351_set_clk_power_enable(si5351_ms_clk_reg_t clk, bool enable);
si5351_err_t si5351_reset_pll();
si5351_err_t si5351_set_output_enable(si5351_ms_clk_reg_t clk, bool enable);
si5351_err_t si5351_set_powerdown();



#ifdef __cplusplus
}
#endif


#endif // _SI5351_H_
