//
// Created by Haydn on 8/20/2026.
//
#include "bme280.h"

static constexpr uint8_t BME280_ADDR        = 0x76 << 1;
static constexpr uint8_t OVERSAMPLE_ONE     = 0b001;
static constexpr uint8_t OVERSAMPLE_SIXTEEN = 0b101;
static constexpr uint8_t REG_CTRL_MEAS      = 0xF4;
static constexpr uint8_t REG_TEMP_MSB       = 0xFA;
static constexpr uint8_t REG_HUM_LSB        = 0xFD;
static constexpr uint8_t REG_CTRL_HUM       = 0xF2;
static constexpr uint8_t REG_DIG_T1         = 0x88;
static constexpr uint8_t REG_DIG_H2         = 0xE1;
static constexpr uint8_t MODE_NORMAL        = 0b00000011;

ErrorCode BME280::start(I2C_HandleTypeDef* hi2c) {
    hi2c_ptr = hi2c;
    // Set mode to normal and enable temperature (for humidity)
    uint8_t powcfg[2];
    powcfg[0] = REG_CTRL_MEAS;
    powcfg[1] = MODE_NORMAL | OVERSAMPLE_SIXTEEN << 5;
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c_ptr, BME280_ADDR, powcfg, 2, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::Failure;
    }

    // Enable Humidity
    uint8_t CTRL_HUM_READ = 0;
    ret = HAL_I2C_Mem_Read(hi2c_ptr, BME280_ADDR, REG_CTRL_HUM,
        I2C_MEMADD_SIZE_8BIT, &CTRL_HUM_READ, 1, HAL_MAX_DELAY );
    if (ret != HAL_OK) {
        return ErrorCode::ErrorRx;
    }

    uint8_t humcfg[2];
    humcfg[0] = REG_CTRL_HUM;
    humcfg[1] = OVERSAMPLE_ONE | CTRL_HUM_READ;
    ret = HAL_I2C_Master_Transmit(hi2c_ptr, BME280_ADDR, humcfg, 2, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::Failure;
    }
    if (readCompReg() != HAL_OK) {
        return ErrorCode::Failure;
    }

    uint8_t id = 0;
    HAL_I2C_Mem_Read(hi2c_ptr, BME280_ADDR, 0xD0, I2C_MEMADD_SIZE_8BIT, &id, 1, HAL_MAX_DELAY);

    return ErrorCode::Success;
}

/// Reads the two HUM registers into the BME280 class
/// @return ErrorCode
ErrorCode BME280::readHumidity() {
    uint8_t buf[16] = {};
    buf[0] = REG_HUM_LSB;
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c_ptr, BME280_ADDR, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::ErrorTx;
    }
    // Read 2 Bytes from HUM_MSB, HUM_LSB (Big Endian)
    ret = HAL_I2C_Master_Receive(hi2c_ptr, BME280_ADDR, buf, 2, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::ErrorRx;
    }
    auto hum16 = static_cast<int16_t>(buf[0] << 8 | buf[1]);
    int32_t val = hum16;
    raw_humidity = val;

    // Humidity is coupled to temperature
    return readTemperature();
}

/// Outputs the humidity
/// @return float
float BME280::getHumidity() {
    (void)compTemp(); // Only need to calc t_fine
    float temp_watch = static_cast<float>(compHum()) / 1024.0f;
    return temp_watch; // Divide by 2^10
}



/*
 * Compensation formulas taken + modified from datasheet
 */

ErrorCode BME280::readTemperature() {
    uint8_t buf[24] = {};
    buf[0] = REG_TEMP_MSB;
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(hi2c_ptr, BME280_ADDR, buf, 1, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::ErrorTx;
    }
    // Read 3 Bytes from TEMP_MSB, TEMP_LSB, TEMP_XLSB (Big Endian)
    ret = HAL_I2C_Master_Receive(hi2c_ptr, BME280_ADDR, buf, 3, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ErrorCode::ErrorRx;
    }
    // temperature read in 20 bits
    const int32_t val = static_cast<int32_t>(buf[0]) << 12 | static_cast<int32_t>(buf[1]) << 4 | (buf[2] & 0xF0) >> 4;
    raw_temp = val;
    return ErrorCode::Success;
}

HAL_StatusTypeDef BME280::readCompReg() {
    // Use burst reads to read all DIG registers
    constexpr uint32_t TPBufSize = 26;
    constexpr uint32_t HBufSize = 7;

    uint8_t digTPBuf[TPBufSize] = {};   // T1 thru T3, H1 at end
    uint8_t digHBuf[HBufSize] = {};     // H2 thru H7

    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(hi2c_ptr, BME280_ADDR, REG_DIG_T1,
                                    I2C_MEMADD_SIZE_8BIT, digTPBuf, TPBufSize, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ret;
    }

    ret = HAL_I2C_Mem_Read(hi2c_ptr, BME280_ADDR, REG_DIG_H2,
                                    I2C_MEMADD_SIZE_8BIT, digHBuf, HBufSize, HAL_MAX_DELAY);
    if (ret != HAL_OK) {
        return ret;
    }

    dig_T1 = static_cast<uint16_t>(digTPBuf[1] << 8 | digTPBuf[0]);
    dig_T2 = static_cast<int16_t>(digTPBuf[3] << 8 | digTPBuf[2]);
    dig_T3 = static_cast<int16_t>(digTPBuf[5] << 8 | digTPBuf[4]);

    dig_H1 = digTPBuf[25];
    dig_H2 = static_cast<int16_t>(digHBuf[1] << 8 | digHBuf[0]);
    dig_H3 = digHBuf[2];
    dig_H4 = static_cast<int16_t>(((digHBuf[3] << 4 ) | (digHBuf[4] & 0b1111)) & 0x0FFF);
    dig_H5 = static_cast<int16_t>((digHBuf[5] << 4 | (digHBuf[4] >> 4)) & 0x0FFF);
    // 12-Bit Conversion
    if (dig_H4 & 0x0800) {
        dig_H4 |= static_cast<int16_t>(0xF000);
    }
    if (dig_H5 & 0x0800) {
        dig_H5 |= static_cast<int16_t>(0xF000);
    }
    dig_H6 = static_cast<int8_t>(digHBuf[6]);

    return HAL_OK;
}

/// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC
int32_t BME280::compTemp()
{
    const int32_t var1 = ((((raw_temp >> 3) - (static_cast<int32_t>(dig_T1) << 1))) * static_cast<int32_t>(dig_T2)) >> 11;
    const int32_t var2 = (((((raw_temp >> 4) - static_cast<int32_t>(dig_T1)) * ((raw_temp >> 4) - static_cast<int32_t>(dig_T1)))
                     >> 12) *
                    static_cast<int32_t>(dig_T3)) >> 14;
    t_fine = var1 + var2;
    const int32_t T = (t_fine * 5 + 128) >> 8;
    return T;
}

/// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 frac bits).
/// Output value of “47445” represents 47445/1024 = 46.333 %RH
uint32_t BME280::compHum()
{
    int32_t v_x1_u32r = (t_fine - static_cast<int32_t>(76800));
    v_x1_u32r = (((((raw_humidity << 14) - (static_cast<int32_t>(dig_H4) << 20) - (static_cast<int32_t>(dig_H5) *
        v_x1_u32r)) + static_cast<int32_t>(16384)) >> 15) * (((((((v_x1_u32r *
        static_cast<int32_t>(dig_H6)) >> 10) * (((v_x1_u32r * static_cast<int32_t>(dig_H3)) >> 11) +
        static_cast<int32_t>(32768))) >> 10) + static_cast<int32_t>(2097152)) * static_cast<int32_t>(dig_H2) +
        8192) >> 14));
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) *
        static_cast<int32_t>(dig_H1)) >> 4));
        v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
        v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
    return static_cast<uint32_t>(v_x1_u32r >> 12);
}