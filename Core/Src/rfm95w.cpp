//
// Created by Haydn on 10/10/2026.
//

#include "rfm95w.h"

static constexpr uint8_t REG_FIFO       = 0x00;
static constexpr uint8_t REG_OPMODE     = 0x01;

static constexpr uint8_t MODE_SLEEP         = 0x07;
static constexpr uint8_t MODE_STANDBY       = 0b001;
static constexpr uint8_t MODE_TX            = 0b011;
static constexpr uint8_t MODE_RXC           = 0b101;
static constexpr uint8_t MODE_RXS           = 0b110;
static constexpr uint8_t MODE_CAD           = 0b111;
static constexpr uint8_t MODE_LORA          = 0x80;

// 915Mhz compensation
static constexpr uint8_t REG_FR_MSB     = 0x06;
static constexpr uint8_t REG_FR_MID     = 0x07;
static constexpr uint8_t REG_FR_LSB     = 0x08;
static constexpr uint8_t VAL_FR_MSB     = 0b11100100;
static constexpr uint8_t VAL_FR_MID     = 0b11000000;
static constexpr uint8_t VAL_FR_LSB      = 0b00000000;

// Config
static constexpr uint8_t REG_MCONF1     = 0x1D;
static constexpr uint8_t REG_MCONF2     = 0x1E;
static constexpr uint8_t REG_MCONF3     = 0x26;
static constexpr uint8_t SET_CRC        = 0b100;
static constexpr uint8_t REG_PACONF     = 0x09;
static constexpr uint8_t SET_PABOOST    = 0x80;


// Todo: add error reporting
uint8_t RFM95W::readRegister(const uint8_t addr) {
    if (hspi_ptr == nullptr) {
        return 0x00;
    }
    uint8_t tx[2] = {static_cast<uint8_t>(addr & 0x7F), 0x00};
    uint8_t rx[2] = {0,0};

    resetCS();
    HAL_SPI_TransmitReceive(hspi_ptr, tx, rx, 2, HAL_MAX_DELAY);
    setCS();

    return rx[1];
}

uint8_t RFM95W::writeRegister(const uint8_t addr, const uint8_t val) {
    if (hspi_ptr == nullptr) {
        return 67;
    }
    uint8_t tx[2] = {static_cast<uint8_t>(addr | 0x80), val};
    uint8_t rx[2] = {0,0};

    resetCS();
    HAL_SPI_TransmitReceive(hspi_ptr, tx, rx, 2, HAL_MAX_DELAY);
    setCS();

    return rx[1];
}

void RFM95W::reset() {
    HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

/// Set RFM95W to LoRa mode at 915MHz
/// @param hspi
/// @return
ErrorCode RFM95W::start(SPI_HandleTypeDef* hspi) {
    hspi_ptr = hspi;
    reset();

    // Enable LoRa + Standby
    uint8_t opmode = readRegister(REG_OPMODE);
    writeRegister(REG_OPMODE, opmode & ~MODE_SLEEP);
    opmode &= ~MODE_SLEEP;
    writeRegister(REG_OPMODE, opmode | MODE_LORA);
    opmode |= MODE_LORA;
    writeRegister(REG_OPMODE, opmode | MODE_STANDBY);
    opmode |= MODE_STANDBY;
    buf = readRegister(REG_OPMODE);

    // Set Freq to 915MHz
    writeRegister(REG_FR_MSB, VAL_FR_MSB);
    writeRegister(REG_FR_MID, VAL_FR_MID);
    writeRegister(REG_FR_LSB, VAL_FR_LSB);

    // Set RxPayloadCrcOn to 1
    uint8_t modemConfig2 = readRegister(REG_MCONF2);
    writeRegister(REG_MCONF2, modemConfig2 | SET_CRC);

    // Set PaSelect to PA_BOOST pin
    uint8_t paConfig = readRegister(REG_PACONF);
    writeRegister(REG_PACONF, paConfig | SET_PABOOST);

    return ErrorCode::Success;
}

uint8_t RFM95W::getBuf() {
    return buf;
}