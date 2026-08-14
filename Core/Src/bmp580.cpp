//
// Created by Haydn on 8/13/2026.
//
#include "bmp580.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

#include <cstdio>
#include <cstring>
#include <sys/_stdint.h>

static constexpr uint8_t BMP580_ADDR            = 0x47 << 1;
static constexpr uint8_t REG_TEMP_DATA_XLSB     = 0x1D;
static constexpr uint8_t REG_ODR_CONFIG         = 0x37;

void BMP580::start(I2C_HandleTypeDef* hi2c) {
    hi2c_ptr = hi2c;
    uint8_t cfg[2];
    cfg[0] = REG_ODR_CONFIG;
    cfg[1] = 0x01;
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c_ptr, BMP580_ADDR, cfg, 2, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        strcpy(reinterpret_cast<char *>(buf), "Error Starting!\r\n");
    } else {
        strcpy(reinterpret_cast<char *>(buf), "BMP580 Started!\r\n");
    }
    osDelay(5);
}

bool BMP580::readTemperature() {
    // Want to read from TEMP_DATA register
    buf[0] = REG_TEMP_DATA_XLSB;
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c_ptr, BMP580_ADDR, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        strcpy(reinterpret_cast<char *>(buf), "Error Tx\r\n");
        return false;
    } else {
        // Read 3 Bytes from TEMP_DATA
        ret = HAL_I2C_Master_Receive(hi2c_ptr, BMP580_ADDR, buf, 3, HAL_MAX_DELAY);
        if (ret != HAL_OK) {
            strcpy(reinterpret_cast<char *>(buf), "Error Rx\r\n");
            return false;
        } else {
            // Combine 24-bits (HIGH << 16, MID << 8, LOW << 0))
            int32_t val = static_cast<int32_t>(buf[2]) << 16 | static_cast<int32_t>(buf[1]) << 8 | buf[0];
            // Get negative
            val = val << 8;
            // Shift back (8 + 16)
            val = val >> 8;
            raw_temperature = val;
            return true;
        }
    }
}

float BMP580::getTemperature() {
    return static_cast<float>(raw_temperature) / 65536.0f; // divide by 2^16
}

uint8_t* BMP580::getBuffer() {
    return buf;
}