//
// Created by Haydn on 8/13/2026.
//
#include "bmp580.h"
#include "stm32f4xx_hal.h"

#include <cstdint>
#include "helpers.h"

static constexpr uint8_t BMP580_ADDR            = 0x47 << 1; // SDO = 1
static constexpr uint8_t REG_TEMP_DATA_XLSB     = 0x1D;
static constexpr uint8_t REG_PRES_DATA_XLSB     = 0x20;
static constexpr uint8_t REG_OSR_CONFIG         = 0x36;
static constexpr uint8_t REG_ODR_CONFIG         = 0x37;
static constexpr uint8_t OSR_PRESS_EN           = 0b01000000;

/// Sets the BMP580 power state to NORMAL
/// @param hi2c i2c pointer
/// @return ErrorCode
ErrorCode BMP580::start(I2C_HandleTypeDef* hi2c) {
    hi2c_ptr = hi2c;
    // Power on (set in odr_config reg)
    uint8_t powcfg[2];
    powcfg[0] = REG_ODR_CONFIG;
    powcfg[1] = 0x01;
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c_ptr, BMP580_ADDR, powcfg, 2, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::Failure;
    }
    // Enable Pressure + Temperature (set press_en in osr_config reg)
    uint8_t OSR_CONFIG = 0;
    ret = HAL_I2C_Mem_Read(hi2c_ptr, BMP580_ADDR, REG_OSR_CONFIG,
        I2C_MEMADD_SIZE_8BIT, &OSR_CONFIG, 1, HAL_MAX_DELAY );
    if (ret != HAL_OK) {
        return ErrorCode::ErrorRx;
    }

    uint8_t osrcfg[2];
    osrcfg[0] = REG_OSR_CONFIG;
    osrcfg[1] = OSR_PRESS_EN | OSR_CONFIG;
    ret = HAL_I2C_Master_Transmit(hi2c_ptr, BMP580_ADDR, osrcfg, 2, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::Failure;
    }
    return ErrorCode::Success;
}

/// Reads the TEMP_DATA register into the BMP580 class
/// @return ErrorCode
ErrorCode BMP580::readTemperature() {
    uint8_t buf[24] = {};
    buf[0] = REG_TEMP_DATA_XLSB;
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c_ptr, BMP580_ADDR, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::ErrorTx;
    }
    // Read 3 Bytes from TEMP_DATA
    ret = HAL_I2C_Master_Receive(hi2c_ptr, BMP580_ADDR, buf, 3, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::ErrorRx;
    }
    // Combine 24-bits (HIGH << 16, MID << 8, LOW << 0))
    int32_t val = static_cast<int32_t>(buf[2]) << 16 | static_cast<int32_t>(buf[1]) << 8 | buf[0];
    // Get negative
    val = val << 8;
    // Shift back (8 + 16)
    val = val >> 8;
    raw_temperature = val;
    return ErrorCode::Success;
}

/// Reads the PRES_DATA register into the BMP580 class
/// @return ErrorCode
ErrorCode BMP580::readPressure() {
    uint8_t buf[24] = {};
    buf[0] = REG_PRES_DATA_XLSB;
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c_ptr, BMP580_ADDR, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::ErrorTx;
    }
    // Read 3 Bytes from PRES_DATA
    ret = HAL_I2C_Master_Receive(hi2c_ptr, BMP580_ADDR, buf, 3, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::ErrorRx;
    }
    // Combine 24-bits (HIGH << 16, MID << 8, LOW << 0))
    int32_t val = static_cast<int32_t>(buf[2]) << 16 | static_cast<int32_t>(buf[1]) << 8 | buf[0];
    // Get negative
    val = val << 8;
    // Shift back (8 + 16)
    val = val >> 8;
    raw_pressure = val;
    return ErrorCode::Success;
}

/// Outputs the temperature in Celsius
/// @return float
float BMP580::getTemperature() {
    return static_cast<float>(raw_temperature) / 65536.0f; // divide by 2^16
}

/// Outputs the pressure in kPa
/// @return float
float BMP580::getPressure() {
    return static_cast<float>(raw_pressure) / 64000.0f; // divide by 2^6 * 1000
}