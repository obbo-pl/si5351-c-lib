/*
 * s5351a-test.ino
 *
 * Created on: 26 lip 2022
 *     Author: Krzysztof Markiewicz <obbo.pl>
 *
 * MIT License
 *
 * Copyright (c) 2022 Krzysztof Markiewicz
 */

#include "si5351.h"
#include "i2c_master.h"
#include "Wire.h"

// function prototype
void show_chip_status(uint8_t status);

/* Test setup
 * ---------------------
 * SI5351A-B-GT
 * XTAL:  25MHz
 * Channel 0: 2650 Hz
 * Channel 1: 2650 Hz Inverted
 * Channel 2: 2343.75 kHz
 */
 
char buffer[60];
si5351_err_t err;
uint8_t status;

void setup() {
    Serial.begin(115200);
    i2c_master_init();
    // Initialization
    err = si5351_init(SI5351_VARIANT_A_B_GT, SI5351_I2C_ADDR_0, SI5351_CRYSTAL_FREQ_25MHZ, 0, false);
    if (err != SI5351_OK) sprintf(buffer, "Init failed: error code(%i)", (int)err);
    Serial.println(buffer);
    // Status check before configuration
    err = si5351_get_status(&status);
    if (err == SI5351_OK) {
        show_chip_status(status);
    } else {
        sprintf(buffer, "Status failed: error code(%i)", (int)err);
        Serial.println(buffer);
    }
    Serial.println();
    
    // Disable output
    // Power down output driver
    si5351_set_powerdown();
    // Write new configuration. This step also powers up the output drivers. Registers 15-92 and 149-170 and 183, 187
    // The order of register configuration is important, go with the flow
    si5351_set_pll_source(SI5351_PLL_XTAL, SI5351_PLL_XTAL, SI5351_CLKIN_DIVIDER1);
    si5351_set_pll_vco(SI5351_PLLA, 600000000);
    // Configuring the multisynth stage
    si5351_set_fanout(false, false, true);
    si5351_set_multisynth(SI5351_MS_CLK0, SI5351_PLLA, 339200);
    si5351_set_multisynth_integer(SI5351_MS_CLK2, SI5351_PLLA, 4);
    // Any unused clock outputs should be powered down
    si5351_set_clk(SI5351_MS_CLK0, true, false, SI5351_CLK_SOURCE_MS_X, SI5351_CLK_R_DIVIDER_128, SI5351_DRIVE_STRENGTH_2mA);
    si5351_set_clk(SI5351_MS_CLK1, true, true, SI5351_CLK_SOURCE_MS_0_OR_4, SI5351_CLK_R_DIVIDER_128, SI5351_DRIVE_STRENGTH_2mA);
    si5351_set_clk(SI5351_MS_CLK2, true, false, SI5351_CLK_SOURCE_MS_X, SI5351_CLK_R_DIVIDER_64, SI5351_DRIVE_STRENGTH_2mA);
    // Apply PLLA and PLLB soft reset
    si5351_reset_pll();
    // Enable desired outputs
    si5351_set_output_enable(SI5351_MS_CLK0, true);
    si5351_set_output_enable(SI5351_MS_CLK1, true);
    si5351_set_output_enable(SI5351_MS_CLK2, true);
}

void loop() {
    err = si5351_get_status(&status);
    if (err == SI5351_OK) {
        show_chip_status(status);
    } else {
        sprintf(buffer, "Status failed: error code(%i)", (int)err);
        Serial.println(buffer);
    }
    Serial.println();
    delay(60000);
}

void show_chip_status(uint8_t status) {
    Serial.println("Si5351 DEVICE STATUS");
    sprintf(buffer, "    SYS_INIT  : %s ", (status & SI5351_DEVICE_STATUS_SYS_INIT_bm) ? "Device is in system initialization mode" : "Device is ready");
    Serial.println(buffer);
    sprintf(buffer, "    LOL_B     : %s ", (status & SI5351_DEVICE_STATUS_LOL_B_bm) ? "Unlocked" : "Locked");
    Serial.println(buffer);
    sprintf(buffer, "    LOL_A     : %s ", (status & SI5351_DEVICE_STATUS_LOL_A_bm) ? "Unlocked" : "Locked");
    Serial.println(buffer);
    sprintf(buffer, "    LOS_CLKIN : %s ", (status & SI5351_DEVICE_STATUS_LOS_CLKIN_bm) ? "Loss": "Valid");
    Serial.println(buffer);
    sprintf(buffer, "    LOS_XTAL  : %s ", (status & SI5351_DEVICE_STATUS_LOS_XTAL_bm) ? "Loss": "Valid");
    Serial.println(buffer);
    sprintf(buffer, "    REVID     : %i ", (status & SI5351_DEVICE_STATUS_REVID_bm));
    Serial.println(buffer);
}
