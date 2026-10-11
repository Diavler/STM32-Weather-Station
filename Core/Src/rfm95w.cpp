//
// Created by Haydn on 10/10/2026.
//

#include "rfm95w.h"
#include <array>

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
static constexpr uint8_t REG_DIOMAP1    = 0x40;
static constexpr uint8_t SET_DIO0TX     = 0x60;

// TX / RX
static constexpr uint8_t MAX_PAYSIZE    = 0x20; // 32 bytes payload
static constexpr uint8_t REG_FIFO       = 0x00;
static constexpr uint8_t REG_IRQ_FLAGS  = 0x12;
static constexpr uint8_t MASK_TXDONE    = 0x08;
static constexpr uint8_t REG_PAYLEN     = 0x22;


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
        return 0x00;
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

    // Set DIO0/G0 Mapping to TxDone
    uint8_t regDioMapping = readRegister(REG_DIOMAP1);
    writeRegister(REG_DIOMAP1, regDioMapping | SET_DIO0TX);

    return ErrorCode::Success;
}

/// Transmit a buffer with len bytes
/// @param buf data to transmit
/// @param len # of bytes to transmit
/// @return
ErrorCode RFM95W::transmit(std::span<uint8_t> buf) {
    if (hspi_ptr == nullptr || buf.size() > MAX_PAYSIZE) {
        return ErrorCode::ErrorTx;
    }
    // Clear IRQ
    writeRegister(REG_IRQ_FLAGS, 0xFF);

    // Assemble buffer
    std::array<uint8_t, MAX_PAYSIZE + 1> tx{};
    tx[0] = static_cast<uint8_t>(REG_FIFO | 0x80);
    std::copy(buf.begin(), buf.end(), tx.begin()+1);

    // Set TX mode
    auto opmode = readRegister(REG_OPMODE);
    writeRegister(REG_OPMODE, opmode | MODE_TX);

    // Transmit
    writeRegister(REG_PAYLEN, tx.size()-1);
    resetCS();
    HAL_SPI_Transmit(hspi_ptr, tx.data(), tx.size(), HAL_MAX_DELAY);
    setCS();

    return ErrorCode::Success;
}

ErrorCode RFM95W::clearIRQ() {
    const uint8_t irq_flags = readRegister(REG_IRQ_FLAGS);
    const bool txdoneSet = irq_flags & MASK_TXDONE;
    writeRegister(REG_IRQ_FLAGS, irq_flags | MASK_TXDONE);
    return txdoneSet ? ErrorCode::Success : ErrorCode::ErrorTx;
}

/// Read into a buffer
/// @param buf buffer to read into
/// @return
ErrorCode RFM95W::receive(std::span<uint8_t> buf) {
    return ErrorCode::Success;
}