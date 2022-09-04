/*
 * si5351_def.h
 *
 * Created on: 27 lip 2022
 *     Author: Krzysztof Markiewicz <obbo.pl>
 *
 * MIT License
 *
 * Copyright (c) 2022 Krzysztof Markiewicz
 */

#ifndef _SI5351_DEF_H_
#define _SI5351_DEF_H_


enum si5351_variant {
    SI5351_VARIANT_A_A_GM,                          // Si5351, Crystal In, Revision A, 20-QFN
    SI5351_VARIANT_A_A_GU,                          // Si5351, Crystal In, Revision A, 24-QSOP
    SI5351_VARIANT_A_A_GT,                          // Si5351, Crystal In, Revision A, 10-MSOP
    SI5351_VARIANT_B_A_GM,                          // Si5351, Crystal In + VCXO, Revision A, 20-QFN
    SI5351_VARIANT_B_A_GU,                          // Si5351, Crystal In + VCXO, Revision A, 24-QSOP
    SI5351_VARIANT_C_A_GM,                          // Si5351, Crystal In + CLKIN, Revision A, 20-QFN
    SI5351_VARIANT_C_A_GU,                          // Si5351, Crystal In + CLKIN, Revision A, 24-QSOP
    SI5351_VARIANT_A_B_GM,                          // Si5351, Crystal In, Revision B, 20-QFN
    SI5351_VARIANT_A_B_GM1,                         // Si5351, Crystal In, Revision B, 16-QFN
    SI5351_VARIANT_A_B_GT,                          // Si5351, Crystal In, Revision B, 10-MSOP
    SI5351_VARIANT_B_B_GM,                          // Si5351, Crystal In + VCXO, Revision B, 20-QFN
    SI5351_VARIANT_B_B_GM1,                         // Si5351, Crystal In + VCXO, Revision B, 16-QFN
    SI5351_VARIANT_C_B_GM,                          // Si5351, Crystal In + CLKIN, Revision B, 20-QFN
    SI5351_VARIANT_C_B_GM1,                         // Si5351, Crystal In + CLKIN, Revision B, 16-QFN
};

#define SI5351_I2C_ADDR_0                           0x60  // All
#define SI5351_I2C_ADDR_1                           0x61  // Si5351A 20-QFN, 24-QSOP, 16-QFN only
                                                    
#define SI5351_POWERUP_TIME_ms                      (10)
#define SI5351_PLL_VCO_MIN                          (600000000UL)
#define SI5351_PLL_VCO_MAX                          (900000000UL)
#define SI5351_CLKIN_MIN                            (10000000UL)
#define SI5351_CLKIN_MAX                            (100000000UL)
#define SI5351_CLKIN_DIVIDER_MAX                    (8)
#define SI5351_PLL_CLKIN_MIN                        (10000000UL)
#define SI5351_PLL_CLKIN_MAX                        (40000000UL)
#define SI5351_PLL_INT_MIN                          (15)
#define SI5351_PLL_INT_MAX                          (90)
#define SI5351_R_DIVIDER_MAX                        (128)

#define SI5351_REVA_MULTISYNTH_FREQUENCY_MIN        (8000UL)
#define SI5351_REVA_MULTISYNTH_FREQUENCY_MAX        (160000000UL)

#define SI5351_REVB_MULTISYNTH_FREQUENCY_MIN        (2500UL)
#define SI5351_REVB_MULTISYNTH_FREQUENCY_MAX        (200000000UL)

#define SI5351_MULTISYNTH_P1_bm                     (0x3FFFF)
#define SI5351_MULTISYNTH_P2_bm                     (0xFFFFF)
#define SI5351_MULTISYNTH_P3_bm                     (0xFFFFF)

#define SI5351_MULTISYNTH_INT_0_TO_5_DIV4           (4)
#define SI5351_MULTISYNTH_FRAC_0_TO_5_MIN           (8)
#define SI5351_MULTISYNTH_FRAC_0_TO_5_MAX           (2048)
#define SI5351_MULTISYNTH_INT_0_TO_7_MIN            (6)
#define SI5351_MULTISYNTH_INT_0_TO_7_MAX            (254)


#define SI5351_DEVICE_STATUS                        0
enum {
    SI5351_DEVICE_STATUS_REVID_bm                   = 0x03, // Revision [1:0].
    SI5351_DEVICE_STATUS_LOS_XTAL_bm                = 0x08, // Crystal Loss of Signal.
    SI5351_DEVICE_STATUS_LOS_CLKIN_bm               = 0x10, // CLKIN Loss Of Signal (Si5351C Only).
    SI5351_DEVICE_STATUS_LOL_A_bm                   = 0x20, // PLLA Loss Of Lock Status.
    SI5351_DEVICE_STATUS_LOL_B_bm                   = 0x40, // PLLB Loss Of Lock Status.
    SI5351_DEVICE_STATUS_SYS_INIT_bm                = 0x80, // System Initialization Status.
};
#define SI5351_INTERRUPT_STATUS_STICKY              1
#define SI5351_INTERRUPT_STATUS_MASK                2
#define SI5351_OUTPUT_ENABLE_CONTROL                3
#define SI5351_OEB_PIN_ENABLE_CONTROL_MASK          9
#define SI5351_PLL_INPUT_SOURCE                     15
enum {
    SI5351_PLL_INPUT_SOURCE_PLLA_SRC_bm             = 0x04, // Input Source Select for PLLA.
    SI5351_PLL_INPUT_SOURCE_PLLB_SRC_bm             = 0x08, // Input Source Select for PLLB.
    SI5351_PLL_INPUT_SOURCE_CLKIN_DIV_bp            = 6,    // ClKIN Input Divider.
    SI5351_PLL_INPUT_SOURCE_CLKIN_DIV_bm            = 0xC0, // ClKIN Input Divider.
    SI5351_PLL_INPUT_SOURCE_CLKIN_DIV1_bm           = 0x00, // ClKIN Input Divider.
    SI5351_PLL_INPUT_SOURCE_CLKIN_DIV2_bm           = 0x40, // ClKIN Input Divider.
    SI5351_PLL_INPUT_SOURCE_CLKIN_DIV4_bm           = 0x80, // ClKIN Input Divider.
    SI5351_PLL_INPUT_SOURCE_CLKIN_DIV8_bm           = 0xC0, // ClKIN Input Divider.

};
#define SI5351_CLK0_CONTROL                         16
#define SI5351_CLK1_CONTROL                         17
#define SI5351_CLK2_CONTROL                         18
#define SI5351_CLK3_CONTROL                         19
#define SI5351_CLK4_CONTROL                         20
#define SI5351_CLK5_CONTROL                         21
#define SI5351_CLK6_CONTROL                         22
#define SI5351_CLK7_CONTROL                         23
enum {
    SI5351_CLK_CONTROL_CLK_IDRV_bm                  = 0x03, // CLK Output Rise and Fall time / Drive Strength Control.
    SI5351_CLK_CONTROL_CLK_IDRV_2mA_bm              = 0x00, // 2 mA.
    SI5351_CLK_CONTROL_CLK_IDRV_4mA_bm              = 0x01, // 4 mA.
    SI5351_CLK_CONTROL_CLK_IDRV_6mA_bm              = 0x02, // 6 mA.
    SI5351_CLK_CONTROL_CLK_IDRV_8mA_bm              = 0x03, // 8 mA.
    SI5351_CLK_CONTROL_CLK_SRC_bm                   = 0x0C, // Output Clock Input Source.
    SI5351_CLK_CONTROL_CLK_SRC_XTAL_bm              = 0x00, // XTAL as the clock.
    SI5351_CLK_CONTROL_CLK_SRC_CLKIN_bm             = 0x04, // CLKIN as the clock source.
    SI5351_CLK_CONTROL_CLK_SRC_MULTISYNTH_0_OR_4_bm = 0x08, // MultiSynth0 or MultiSynth4 as the source.
    SI5351_CLK_CONTROL_CLK_SRC_MULTISYNTH_X_bm      = 0x0C, // MultiSynthX as the source.
    SI5351_CLK_CONTROL_CLK_INV_bm                   = 0x10, // Output Clock Invert.
    SI5351_CLK_CONTROL_MS_SRC_bm                    = 0x20, // MultiSynth Source Select for CLK.
    SI5351_CLK_CONTROL_MS_INT_bm                    = 0x40, // MultiSynth Integer Mode. CLK0 to CLK5
    SI5351_CLK_CONTROL_FB_INT_bm                    = 0x40, // FBA, FBB MultiSynth Integer Mode.
    SI5351_CLK_CONTROL_CLK_PDN_bm                   = 0x80, // Clock Power Down
};

#define SI5351_CLK3_TO_0_DISABLE_STATE              24
#define SI5351_CLK7_TO_4_DISABLE_STATE              25
enum {
    SI5351_CLK0_TO_7_DISABLE_STATE_CLK_bm           = 0x03,
    SI5351_CLK0_TO_7_DISABLE_STATE_CLK_LOW_bm       = 0x00,
    SI5351_CLK0_TO_7_DISABLE_STATE_CLK_HIGH_bm      = 0x01,
    SI5351_CLK0_TO_7_DISABLE_STATE_CLK_HIGHIMP_bm   = 0x02,
    SI5351_CLK0_TO_7_DISABLE_STATE_CLK_NEVER_bm     = 0x03,
};
#define SI5351_MULTISYNTH_NA_PARAMETERS             26
#define SI5351_MULTISYNTH_NB_PARAMETERS             34
#define SI5351_MULTISYNTH_NX_PARAMETERS_LENGTH      8
#define SI5351_MULTISYNTH0_PARAMETERS               42
#define SI5351_MULTISYNTH1_PARAMETERS               50
#define SI5351_MULTISYNTH2_PARAMETERS               58
#define SI5351_MULTISYNTH3_PARAMETERS               66
#define SI5351_MULTISYNTH4_PARAMETERS               74
#define SI5351_MULTISYNTH5_PARAMETERS               82
#define SI5351_MULTISYNTH_0_TO_5_PARAMETERS_LENGTH  8
enum {
    SI5351_MULTISYNTH0_PARAMETERS_MS_DIV4_bm        = 0x0C,
    SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_bm      = 0x70,
    SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_1_bm    = 0x00,
    SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_2_bm    = 0x10,
    SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_4_bm    = 0x20,
    SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_8_bm    = 0x30,
    SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_16_bm   = 0x40,
    SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_32_bm   = 0x50,
    SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_64_bm   = 0x60,
    SI5351_MULTISYNTH0_PARAMETERS_R_DIVIDER_128_bm  = 0x70,
};
#define SI5351_MULTISYNTH6_PARAMETERS               90
#define SI5351_MULTISYNTH7_PARAMETERS               91
#define SI5351_CLOCK_6_AND_7_OUTPUT_DIVIDER         92

#define SI5351_SPREAD_SPECTRUM_PARAMETERS           149

#define SI5351_CLK0_INITIAL_PHASE_OFFSET            165
#define SI5351_CLK1_INITIAL_PHASE_OFFSET            166
#define SI5351_CLK2_INITIAL_PHASE_OFFSET            167
#define SI5351_CLK3_INITIAL_PHASE_OFFSET            168
#define SI5351_CLK4_INITIAL_PHASE_OFFSET            169
#define SI5351_CLK5_INITIAL_PHASE_OFFSET            170
enum {
    SI5351_CLK_INITIAL_PHASE_OFFSET_bm              = 0x7F
};
#define SI5351_PLL_RESET                            177
enum {
    SI5351_PLL_RESET_PLLA_RST_bm                    = 0x20, // PLLA_Reset.
    SI5351_PLL_RESET_PLLB_RST_bm                    = 0x80, // PLLB_Reset.
};
#define SI5351_CRYSTAL_INTERNAL_LOAD_CAP            183
enum {
    SI5351_CRYSTAL_INTERNAL_LOAD_CAP_XTAL_CL_bm     = 0xC0, // Crystal Load Capacitance Selection.
    SI5351_CRYSTAL_INTERNAL_LOAD_CAP_XTAL_6PF_bm    = 0x40, // Internal CL = 6 pF.
    SI5351_CRYSTAL_INTERNAL_LOAD_CAP_XTAL_8PF_bm    = 0x80, // Internal CL = 8 pF.
    SI5351_CRYSTAL_INTERNAL_LOAD_CAP_XTAL_10PF_bm   = 0xC0, // Internal CL = 10 pF (default).
    SI5351_CRYSTAL_INTERNAL_LOAD_CAP_RESERVED_bm    = 0x12, // Bits 5:0 should be written to 010010b.
};
#define SI5351_FANOUT_ENABLE                        187
enum {
    SI5351_FANOUT_ENABLE_MS_bm                      = 0x10,
    SI5351_FANOUT_ENABLE_XO_bm                      = 0x40,
    SI5351_FANOUT_ENABLE_CLKIN_bm                   = 0x80,
};

#endif /* _SI5351_DEF_H_ */
