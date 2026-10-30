//
// Created by Haydn on 10/10/2026.
//

#ifndef WEATHERSTATION_RFM95W_H
#define WEATHERSTATION_RFM95W_H

#include "helpers.h"
#include "stm32f4xx_hal.h"
#include <span>

// Copy pasted from main.h: RST, G0, CS
#define RST_Pin GPIO_PIN_7
#define RST_GPIO_Port GPIOC
#define G0_Pin GPIO_PIN_9
#define G0_GPIO_Port GPIOA
#define G0_EXTI_IRQn EXTI9_5_IRQn
#define CS_Pin GPIO_PIN_6
#define CS_GPIO_Port GPIOB

/// RFM95W LoRa driver
class RFM95W {
public:
    static ErrorCode start(SPI_HandleTypeDef* hspi);
    static ErrorCode transmit(std::span<uint8_t> buf);
    static ErrorCode receive(std::span<uint8_t> buf);
    static ErrorCode clearIRQ();
private:
    inline static SPI_HandleTypeDef* hspi_ptr = nullptr;
    // Todo: change hardcoded GPIO pins to somewhere else
    static void reset();
    static void resetCS() { HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET); }
    static void setCS() { HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET); }
    static uint8_t readRegister(uint8_t addr);
    static uint8_t writeRegister(uint8_t addr, uint8_t val);
};

#endif //WEATHERSTATION_RFM95W_H
