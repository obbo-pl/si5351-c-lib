/*
 * s5351a-test.c
 *
 * Created on: 26 lip 2022
 *     Author: Krzysztof Markiewicz <obbo.pl>
 *
 * MIT License
 *
 * Copyright (c) 2022 Krzysztof Markiewicz
 */

#include <stdio.h>
#include <ctype.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include <driver/gpio.h>
#include "soc/soc.h"
#include <esp_log.h>
#include "driver/i2c.h"
#include "i2c_master.h"
#include "si5351.h"


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

static const char *TAG = "si5351a";
si5351_err_t err;
uint8_t status;

void app_main(void)
{
    i2c_master_init();
    // Initialization
    err = si5351_init(SI5351_VARIANT_A_B_GT, SI5351_I2C_ADDR_0, SI5351_CRYSTAL_FREQ_25MHZ, 0, false);
    if (err != SI5351_OK) ESP_LOGE(TAG, "Init failed: %s", esp_err_to_name(err));
    // Status check before configuration
    err = si5351_get_status(&status);
    if (err == SI5351_OK) {
        show_chip_status(status);
    } else {
        ESP_LOGE(TAG, "Status failed: %s", esp_err_to_name(err));
    }
    printf("\n");

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

    while (1)
    {
        // Status check
        err = si5351_get_status(&status);
        if (err == SI5351_OK) {
            show_chip_status(status);
        } else {
            ESP_LOGE(TAG, "Status failed: %s", esp_err_to_name(err));
        }
        printf("\n");
        vTaskDelay(1000 * 60 / portTICK_PERIOD_MS);
    }
}

void show_chip_status(uint8_t status)
{
    printf("Si5351 DEVICE STATUS \n");
    printf("    SYS_INIT  : %s \n", (status & SI5351_DEVICE_STATUS_SYS_INIT_bm) ? "Device is in system initialization mode" : "Device is ready");
    printf("    LOL_B     : %s \n", (status & SI5351_DEVICE_STATUS_LOL_B_bm) ? "Unlocked" : "Locked");
    printf("    LOL_A     : %s \n", (status & SI5351_DEVICE_STATUS_LOL_A_bm) ? "Unlocked" : "Locked");
    printf("    LOS_CLKIN : %s \n", (status & SI5351_DEVICE_STATUS_LOS_CLKIN_bm) ? "Loss": "Valid");
    printf("    LOS_XTAL  : %s \n", (status & SI5351_DEVICE_STATUS_LOS_XTAL_bm) ? "Loss": "Valid");
    printf("    REVID     : %i \n", (status & SI5351_DEVICE_STATUS_REVID_bm));
}

