//
// Created by Haydn on 8/14/2026.
//

#ifndef WEATHERSTATION_SSD1315_H
#define WEATHERSTATION_SSD1315_H

#include "helpers.h"
#include "stm32f4xx_hal.h"

class SSD1315 {
public:
    static ErrorCode start(I2C_HandleTypeDef* hi2c);
private:
    inline static I2C_HandleTypeDef* hi2c_ptr = nullptr;
};


#endif //WEATHERSTATION_SSD1315_H
