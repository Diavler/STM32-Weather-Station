# STM32 Weather Station
- Built & tested on STM32 446RE
- Contains C++ drivers for BME280, BMP580, and RFM95W.
  - Temperature, Pressure, Humidity
  - Transmit and Receive packets via LoRa
- FreeRTOS used for concurrency and interrupt management (SensorTask, RadioTask)
- The other half of the "station" is an ESP32, which receives the data and broadcasts it via WiFi
## Todo
1. Migrate C data structures to C++ standard library (i.e. std::array)
2. Implement FreeRTOS data structures
3. Complete RFM95W driver functionality
4. Refactor error handling for RFM95W driver (temporary return values as of now)
5. Refactor rfm95w driver to have an interface, so stm32 and esp32 can use their respective HAL.
6. Implement ESP32 recieving end
7. Implement ESP32 UDP transmission on WiFi
8. Implement desktop recieving end
## Building
Install:
- STM32CubeMX
- STM32CubeCLT
- arm-none-eabi-gcc
- arm-none-eabi-gdb
