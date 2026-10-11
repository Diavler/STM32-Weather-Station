//
// Created by Haydn on 8/13/2026.
//
#include "main_ws.h"

#include <array>
#include <cstdio>
#include <cstring>

#include "bme280.h"
#include "bmp580.h"
#include "rfm95w.h"

// Todo: look into CMSIS2 wrappers vs FreeRTOS
#include "cmsis_os2.h"
#include "freertos_os2.h"
#include "task.h"
#include "stm32f4xx_hal.h"

/* Definitions for sensorTask */
osThreadId_t sensorTaskHandle;
constexpr osThreadAttr_t sensorTask_attributes = {
    .name = "sensorTask",
    .stack_size = 1024 * 4,
    .priority = osPriorityHigh,
  };
/* Definitions for radioTask */
static osThreadId_t radioTaskHandle;
constexpr osThreadAttr_t radioTask_attributes = {
    .name = "radioTask",
    .stack_size = 256 * 4,
    .priority = osPriorityNormal,
  };

extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart2;

void main_ws(void) {
    sensorTaskHandle = osThreadNew(StartSensorTask, nullptr, &sensorTask_attributes);
    radioTaskHandle = osThreadNew(StartRadioTask, nullptr, &radioTask_attributes);
}

/// Temporary function to clean up errors. Todo: implement more elegant string / error handling
static void sendUARTErrorTX() {
    const auto msg = errorString(ErrorCode::ErrorTx);
    HAL_UART_Transmit(&huart2, reinterpret_cast<const uint8_t *>(msg), strlen(reinterpret_cast<const char *>(msg)), HAL_MAX_DELAY);
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

    if (BME280::start(&hi2c1) == ErrorCode::Failure) {
        char startFailMsg[] = "ERROR: BME280 failed to start.\r\n";
        HAL_UART_Transmit(&huart2, reinterpret_cast<uint8_t*>(startFailMsg), strlen(startFailMsg), HAL_MAX_DELAY);
        osThreadTerminate(nullptr);
    } else {
        char startMsg[] = "BME280 powered on!\r\n";
        HAL_UART_Transmit(&huart2, reinterpret_cast<uint8_t*>(startMsg), strlen(startMsg), HAL_MAX_DELAY);
    }

    for(;;)
    {
        const auto tempRet = BMP580::readTemperature();
        const auto presRet = BMP580::readPressure();
        const auto humRet   = BME280::readHumidity();

        if (tempRet == ErrorCode::ErrorRx || presRet == ErrorCode::ErrorRx || humRet == ErrorCode::ErrorRx) {
            sendUARTErrorTX();
        }
        if (tempRet == ErrorCode::ErrorTx || presRet == ErrorCode::ErrorTx || humRet == ErrorCode::ErrorTx) {
            sendUARTErrorTX();
        }

        const sensorBundle sensorReading{
            BMP580::getTemperature(), BMP580::getPressure(), BME280::getHumidity()
        };
        char output[64];
        const int len = snprintf(output, sizeof(output), "%.2f C    %.2f kPa    %.1f%% Humidity\r\n",
            sensorReading.temperature, sensorReading.pressure, sensorReading.humidity);
        HAL_UART_Transmit(&huart2, reinterpret_cast<uint8_t*>(output), len, HAL_MAX_DELAY);

        // Todo: implement a FreeRTOS queue that sends [temperature, pressure, humidity]

        osDelay(5000);
    }
    osThreadTerminate(nullptr);
}

void StartRadioTask(void *argument) {
    osDelay(500);
    RFM95W::start(&hspi1);
    BaseType_t taskWoken = pdTRUE;
    constexpr TickType_t notifyTICKS = 250; // Todo: calculate proper value

    uint8_t tempbuf = 0;
    std::array<char, 8> output{};
    const int len = snprintf(output.data(), output.size(), "%d\r\n", tempbuf);
    for (;;) {
        HAL_UART_Transmit(&huart2, reinterpret_cast<uint8_t*>(output.data()), len, HAL_MAX_DELAY);
        xTaskNotifyStateClear(static_cast<TaskHandle_t>(radioTaskHandle));
        std::array<uint8_t, 32> txBuf{};
        txBuf[0] = 0x88; // Todo: replace dummy data with readin from senesor queue
        auto transmitRet = RFM95W::transmit(txBuf);
        if (transmitRet != ErrorCode::Success) {
            sendUARTErrorTX();
        }
        ulTaskNotifyTake(taskWoken, notifyTICKS);
        auto clearRet = RFM95W::clearIRQ();
        if (clearRet != ErrorCode::Success) {
            sendUARTErrorTX();
        }
        osDelay(5000);
    }
    osThreadTerminate(nullptr);
}

extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_9) {
        BaseType_t taskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(static_cast<TaskHandle_t>(radioTaskHandle), &taskWoken);
        portYIELD_FROM_ISR(taskWoken);
    }
}

