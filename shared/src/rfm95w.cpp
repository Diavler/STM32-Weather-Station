//
// Created by Haydn on 10/10/2026.
//

#include "../include/rfm95w.h"
#include "../include/rfm95w_constants.h"
#include <array>
/* Helper Functions */

std::expected<uint8_t, ErrorCode> RFM95W::readRegister(const uint8_t addr) {
    if (hspi_ptr == nullptr) {
        return std::unexpected(ErrorCode::invalidArgs);
    }
    uint8_t tx[2] = {static_cast<uint8_t>(addr & 0x7F), 0x00};
    uint8_t rx[2] = {0,0};

    resetCS();
    const auto halStatus = HAL_SPI_TransmitReceive(hspi_ptr, tx, rx, 2, HAL_MAX_DELAY);
    setCS();
    if (halStatus != HAL_OK) return std::unexpected(ErrorCode::spiError);

    return rx[1];
}

ErrorCode RFM95W::writeRegister(const uint8_t addr, const uint8_t val) {
    if (hspi_ptr == nullptr) {
        return ErrorCode::invalidArgs;
    }
    uint8_t tx[2] = {static_cast<uint8_t>(addr | 0x80), val};
    uint8_t rx[2] = {0,0};

    resetCS();
    const auto halStatus = HAL_SPI_TransmitReceive(hspi_ptr, tx, rx, 2, HAL_MAX_DELAY);
    setCS();
    if (halStatus != HAL_OK) return ErrorCode::spiError;

    return ErrorCode::Success;
}

/* End of Helper Functions */

ErrorCode RFM95W::updateRegister(const uint8_t addr, const uint8_t mask, const uint8_t val) {
    auto read = readRegister(addr);
    if (!read) return read.error();
    // Mask the register, then set the new part of the register
    const auto newVal = (*read & ~mask) | (val & mask);
    return writeRegister(addr, newVal);
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
    if (hspi == nullptr) return ErrorCode::invalidArgs;
    hspi_ptr = hspi;
    reset();

    // Enable LoRa + Standby
    static constexpr std::array<RegUpdate, 3> updateOPMODE {{
        {.addr = REG_OPMODE, .mask = MASK_MODE, .val = MODE_SLEEP},
        {.addr = REG_OPMODE, .mask = MODE_LORA, .val = MODE_LORA},
        {.addr = REG_OPMODE, .mask = MASK_MODE, .val = MODE_STANDBY}
    }};
    for (const auto& [addr, mask, val] : updateOPMODE) {
        if (const auto err = updateRegister(addr, mask, val); err != ErrorCode::Success) return err;
    }
    // Set Frequency to 915MHz
    static constexpr std::array<RegWrite, 3> setFREQ {{
        {.addr = REG_FR_MSB, .val = VAL_FR_MSB},
        {.addr = REG_FR_MID, .val = VAL_FR_MID},
        {.addr = REG_FR_LSB, .val = VAL_FR_LSB},
    }};
    for (const auto& [addr, val] : setFREQ) {
        if (const auto err = writeRegister(addr, val); err != ErrorCode::Success) return err;
    }
    static constexpr std::array<RegUpdate, 3> setMISC {{
        {.addr = REG_MCONF2, .mask = SET_CRC, .val = SET_CRC},          // RxPayloadCrcOn to 1
        {.addr = REG_PACONF, .mask = SET_PABOOST, .val = SET_PABOOST},  // PaSelect to PA_BOOST pin
        {.addr = REG_DIOMAP1, .mask = SET_DIO0TX, .val = SET_DIO0TX},   // DIO0/G0 Mapping to TxDone
    }};
    for (const auto& [addr, mask, val] : setMISC) {
        if (const auto err = updateRegister(addr, mask, val); err != ErrorCode::Success) return err;
    }

    return ErrorCode::Success;
}

/// Transmit a buffer with len bytes
/// @param buf data to transmit
/// @param len # of bytes to transmit
/// @return
ErrorCode RFM95W::transmit(std::span<uint8_t> buf) {
    if (hspi_ptr == nullptr || buf.size() > MAX_PAYSIZE) {
        return ErrorCode::invalidArgs;
    }
    // Clear IRQ
    if (const auto err = writeRegister(REG_IRQ_FLAGS, 0xFF); err != ErrorCode::Success) return err;

    // Assemble buffer
    std::array<uint8_t, MAX_PAYSIZE + 1> tx{};
    tx[0] = static_cast<uint8_t>(REG_FIFO | 0x80);
    std::copy(buf.begin(), buf.end(), tx.begin()+1);

    // Set TX mode
    if (const auto err = updateRegister(REG_OPMODE, MASK_MODE, MODE_TX); err != ErrorCode::Success) return err;

    // Transmit
    if (const auto err = writeRegister(REG_PAYLEN, tx.size()-1); err != ErrorCode::Success) return err;
    resetCS();
    const auto halStatus = HAL_SPI_Transmit(hspi_ptr, tx.data(), tx.size(), HAL_MAX_DELAY);
    setCS();
    if (halStatus != HAL_OK) return ErrorCode::spiError;

    return ErrorCode::Success;
}

ErrorCode RFM95W::clearIRQ() {
    return updateRegister(REG_IRQ_FLAGS, MASK_TXDONE, MASK_TXDONE);
}

/// Read into a buffer
/// @param buf buffer to read into
/// @return
ErrorCode RFM95W::receive(std::span<uint8_t> buf) {
    return ErrorCode::Success;
}