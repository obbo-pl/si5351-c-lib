/*
 * i2c_master.h
 *
 * Created on: 04 gru 2021
 *     Author: Krzysztof Markiewicz <obbo.pl>
 *
 * MIT License
 *
 * Copyright (c) 2021 Krzysztof Markiewicz
 */

#ifndef _I2C_MASTER_H_
#define _I2C_MASTER_H_

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"


#define I2C_PORT_NUM                I2C_NUM_1       // CONFIG_I2C_PORT_NUM
#define I2C_SCL_IO                  GPIO_NUM_22     // CONFIG_GPIO_SCL
#define I2C_SDA_IO                  GPIO_NUM_21     // CONFIG_GPIO_SDA
#define I2C_FREQ_HZ                 100000          // CONFIG_I2C_FREQ_HZ

#define I2C_TX_BUF_DISABLE          0
#define I2C_RX_BUF_DISABLE          0

#define WRITE_BIT                   I2C_MASTER_WRITE
#define READ_BIT                    I2C_MASTER_READ
#define ACK_CHECK_EN                0x1
#define ACK_CHECK_DIS               0x0
#define ACK_VAL                     0x0
#define NACK_VAL                    0x1


esp_err_t i2c_master_init();
esp_err_t i2c_master_read_reg(uint8_t i2c_addr, uint8_t i2c_reg, uint8_t* data_rd, size_t size);
esp_err_t i2c_master_write_reg(uint8_t i2c_addr, uint8_t i2c_reg, uint8_t* data_wr, size_t size);


#endif /* _I2C_MASTER_H_ */
