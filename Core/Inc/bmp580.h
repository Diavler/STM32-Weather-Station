//
// Created by Haydn on 8/13/2026.
//

#ifndef WEATHERSTATION_BMP580_H
#define WEATHERSTATION_BMP580_H

#include <cstdint>
#include "helpers.h"
#include "stm32f4xx_hal.h"

class BMP580 {
public:
    static ErrorCode start(I2C_HandleTypeDef* hi2c);
    static ErrorCode readTemperature();
    //static ErrorCode readPressure();
    //static ErrorCode readHumidity();
    static float getTemperature();
private:
    inline static I2C_HandleTypeDef* hi2c_ptr = nullptr;
    inline static int32_t raw_temperature = 0;
};

#endif //WEATHERSTATION_BMP580_H
