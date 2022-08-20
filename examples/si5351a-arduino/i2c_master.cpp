/*
 * i2c_master.cpp
 *
 * Created on: 26 lip 2022
 *     Author: Krzysztof Markiewicz <obbo.pl>
 *
 * MIT License
 *
 * Copyright (c) 2021 Krzysztof Markiewicz
 */

#include "i2c_master.h"
#include <Arduino.h>
#include "Wire.h"


uint8_t i2c_master_init()
{
    Wire.begin(I2C_SDA_IO, I2C_SCL_IO, I2C_FREQ_HZ);
    return 0;
}

uint8_t i2c_master_read_reg(uint8_t i2c_addr, uint8_t i2c_reg, uint8_t* data_rd, size_t size)
{
    uint8_t result;
    Wire.beginTransmission(i2c_addr);
    Wire.write(i2c_reg);
    result = Wire.endTransmission();
    Wire.requestFrom(i2c_addr, size);
    while(Wire.available() && size) {
        *data_rd = (uint8_t)Wire.read();
        data_rd++;
        size--;
    }
    return result;
}

uint8_t i2c_master_write_reg(uint8_t i2c_addr, uint8_t i2c_reg, uint8_t* data_wr, size_t size)
{
    uint8_t result;
    Wire.beginTransmission(i2c_addr);
    Wire.write(i2c_reg);
    Wire.write(data_wr, size);
    result = Wire.endTransmission();
    return result;
}
