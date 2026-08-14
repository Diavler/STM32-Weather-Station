//
// Created by Haydn on 8/13/2026.
//
#include "main_ws.h"

#include <cstdio>
#include <cstring>

#include "bmp580.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

/* Definitions for sensorTask */
osThreadId_t sensorTaskHandle;
constexpr osThreadAttr_t sensorTask_attributes = {
    .name = "sensorTask",
    .stack_size = 1024 * 4,
    .priority = osPriorityHigh,
  };
/* Definitions for displayTask */
static osThreadId_t displayTaskHandle;
constexpr osThreadAttr_t displayTask_attributes = {
    .name = "displayTask",
    .stack_size = 256 * 4,
    .priority = osPriorityNormal,
  };

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;

void main_ws(void) {
    sensorTaskHandle = osThreadNew(StartSensorTask, nullptr, &sensorTask_attributes);
    //displayTaskHandle = osThreadNew(StartDisplayTask, nullptr, &displayTask_attributes);
}

void StartSensorTask(void *argument) {
    // TODO: implement cleaner error codes + handling
    if (BMP580::start(&hi2c1) == ErrorCode::Failure) {
        char startFailMsg[] = "ERROR: BMP580 failed to start.\r\n";
        HAL_UART_Transmit(&huart2, reinterpret_cast<uint8_t*>(startFailMsg), strlen(startFailMsg), HAL_MAX_DELAY);
        osThreadTerminate(nullptr);
    } else {
        char startMsg[] = "BMP580 powered on!\r\n";
        HAL_UART_Transmit(&huart2, reinterpret_cast<uint8_t*>(startMsg), strlen(startMsg), HAL_MAX_DELAY);
    }

    for(;;)
    {
        const auto ret = BMP580::readTemperature();
        if (ret == ErrorCode::Success) {
            const float temperature = BMP580::getTemperature();
            char output[32];
            const int len = snprintf(output, sizeof(output), "%.2f C\r\n", temperature);
            HAL_UART_Transmit(&huart2, reinterpret_cast<uint8_t*>(output), len, HAL_MAX_DELAY);
        }
        if (ret == ErrorCode::ErrorRx) {
            const auto msg = errorString(ret);
            HAL_UART_Transmit(&huart2, reinterpret_cast<const uint8_t *>(msg), strlen(reinterpret_cast<const char *>(msg)), HAL_MAX_DELAY);
        }
        if (ret == ErrorCode::ErrorRx) {
            const auto msg = errorString(ret);
            HAL_UART_Transmit(&huart2, reinterpret_cast<const uint8_t *>(msg), strlen(reinterpret_cast<const char *>(msg)), HAL_MAX_DELAY);
        }
        osDelay(500);
    }
    osThreadTerminate(nullptr);
}

void StartDisplayTask(void *argument) {
    osThreadTerminate(nullptr);
}