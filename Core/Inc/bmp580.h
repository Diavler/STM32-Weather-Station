//
// Created by Haydn on 8/13/2026.
//

#ifndef WEATHERSTATION_BMP580_H
#define WEATHERSTATION_BMP580_H
#include <sys/_stdint.h>
#include "stm32f4xx_hal.h"

class BMP580 {
public:
    static void start(I2C_HandleTypeDef* hi2c);
    static bool readTemperature();
    //static float getPressure();
    //static float getHumidity();
    static uint8_t* getBuffer();
    static float getTemperature();
private:
    inline static I2C_HandleTypeDef* hi2c_ptr = nullptr;
    inline static uint8_t buf[24] = {};
    inline static int32_t raw_temperature = 0;
};

#endif //WEATHERSTATION_BMP580_H
