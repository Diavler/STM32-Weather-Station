# STM32 Weather Station
- Built & tested on STM32 446RE
- Contains C++ drivers for BME280, BMP580, and RFM95W.
  - Temperature, Pressure, Humidity
  - Transmit and Receive packets via LoRa
- FreeRTOS used for concurrency and interrupt management (SensorTask, RadioTask)
- The other half of the "station" is an ESP32, which receives the data and broadcasts it via WiFi
## WIP
- RFM95W transmission
## Todo
- Migrate C data structures to C++ standard library (i.e. std::array)
- Refactor error handling for RFM95W driver (temporary return values as of now)
- Implement FreeRTOS data structures
- ESP32 receiving end (seperate repository)