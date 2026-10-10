//
// Created by Haydn on 10/10/2026.
//

#ifndef WEATHERSTATION_RFM95W_H
#define WEATHERSTATION_RFM95W_H

#include "helpers.h"
#include "stm32f4xx_hal.h"

/// RFM95W LoRa driver
class RFM95W {
public:
    static ErrorCode start(SPI_HandleTypeDef* hspi);
private:
    inline static SPI_HandleTypeDef* hspi_ptr = nullptr;

    // Internal Data

};

#endif //WEATHERSTATION_RFM95W_H
