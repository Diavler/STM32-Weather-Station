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
    .priority = (osPriority_t) osPriorityHigh,
  };
/* Definitions for displayTask */
static osThreadId_t displayTaskHandle;
constexpr osThreadAttr_t displayTask_attributes = {
    .name = "displayTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t) osPriorityNormal,
  };

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart2;

void main_ws(void) {
    sensorTaskHandle = osThreadNew(StartSensorTask, nullptr, &sensorTask_attributes);
    //displayTaskHandle = osThreadNew(StartDisplayTask, nullptr, &displayTask_attributes);
}

void StartSensorTask(void *argument) {
    BMP580::start(&hi2c1);
    uint8_t* buf = BMP580::getBuffer();
    HAL_UART_Transmit(&huart2, buf, strlen(reinterpret_cast<char *>(buf)), HAL_MAX_DELAY);
    for(;;)
    {
        BMP580::readTemperature();
        float temperature = BMP580::getTemperature();
        char output[32];
        int len = snprintf(output, sizeof(output), "%.2f C\r\n", temperature);
        HAL_UART_Transmit(&huart2, reinterpret_cast<uint8_t*>(output), len, HAL_MAX_DELAY);
        osDelay(500);
    }

    // In case we accidentally exit from task loop
    osThreadTerminate(nullptr);
}

void StartDisplayTask(void *argument) {
    osThreadTerminate(nullptr);
}