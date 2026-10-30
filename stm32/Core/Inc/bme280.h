//
// Created by Haydn on 8/20/2026.
//

#ifndef WEATHERSTATION_BME280_H
#define WEATHERSTATION_BME280_H

#include <cstdint>
#include "../../../shared/include/helpers.h"
#include "stm32f4xx_hal.h"

/// I2C Humidity Sensor
class BME280 {
public:
    static ErrorCode start(I2C_HandleTypeDef* hi2c);
    static ErrorCode readHumidity();
    static float getHumidity();
private:
    static int32_t compTemp();
    static uint32_t compHum();
    static ErrorCode readTemperature();
    static HAL_StatusTypeDef readCompReg();
    inline static I2C_HandleTypeDef* hi2c_ptr = nullptr;
    inline static int32_t raw_humidity = 0;

    // Compensation / Calibration data
    inline static int32_t       raw_temp = 0;
    inline static int32_t       t_fine = 0;

    inline static uint16_t      dig_T1 = 0;
    inline static int16_t       dig_T2 = 0;
    inline static int16_t       dig_T3 = 0;

    inline static uint8_t       dig_H1 = 0;
    inline static int16_t       dig_H2 = 0;
    inline static uint8_t       dig_H3 = 0;
    inline static int16_t       dig_H4 = 0;
    inline static int16_t       dig_H5 = 0;
    inline static int8_t        dig_H6 = 0;
};

#endif //WEATHERSTATION_BME280_H
