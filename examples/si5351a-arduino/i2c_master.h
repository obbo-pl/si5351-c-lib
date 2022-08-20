/*
 * i2c_master.h
 *
 * Created on: 26 lip 2022
 *     Author: Krzysztof Markiewicz <obbo.pl>
 *
 * MIT License
 *
 * Copyright (c) 2021 Krzysztof Markiewicz
 */ 

#ifndef _I2C_MASTER_H_
#define _I2C_MASTER_H_

#include <stdbool.h>
#include <stdio.h>

#define I2C_PORT_NUM                I2C_NUM_1          // CONFIG_I2C_PORT_NUM
#define I2C_SCL_IO                  GPIO_NUM_22        // CONFIG_GPIO_SCL
#define I2C_SDA_IO                  GPIO_NUM_21        // CONFIG_GPIO_SDA
#define I2C_FREQ_HZ                 100000             // CONFIG_I2C_FREQ_HZ



#ifdef __cplusplus
extern "C" {
#endif


uint8_t i2c_master_init();
uint8_t i2c_master_read_reg(uint8_t i2c_addr, uint8_t i2c_reg, uint8_t* data_rd, size_t size);
uint8_t i2c_master_write_reg(uint8_t i2c_addr, uint8_t i2c_reg, uint8_t* data_wr, size_t size);


#ifdef __cplusplus
}
#endif


#endif /* _I2C_MASTER_H_ */
