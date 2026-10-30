//
// Created by Haydn on 10/10/2026.
//

#include "../include/rfm95w.h"
#include "../include/rfm95w_constants.h"
#include <array>
/* Helper Functions */

std::expected<uint8_t, ErrorCode> RFM95W::readRegister(const uint8_t addr) const
{
    constexpr auto spiSize = 2;
    std::array<uint8_t, spiSize> tx{static_cast<uint8_t>(addr & 0x7F), 0x00};
    std::array<uint8_t, spiSize> rx{0,0};

    const auto err = m_interface.spiTransfer(tx, rx);
    if (err != ErrorCode::Success) return std::unexpected(err);

    return rx[1];
}

ErrorCode RFM95W::writeRegister(const uint8_t addr, const uint8_t val) {
    constexpr auto spiSize = 2;
    std::array<uint8_t, spiSize> tx{static_cast<uint8_t>(addr | 0x80), 0x00};

    const auto err = m_interface.spiTransfer(tx, {});
    if (err != ErrorCode::Success) return err;

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

/// Set RFM95W to LoRa mode at 915MHz
/// @return
ErrorCode RFM95W::start() {
    m_interface.reset();

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
/// @return
ErrorCode RFM95W::transmit(std::span<uint8_t> buf) {
    // Clear IRQ
    if (const auto err = writeRegister(REG_IRQ_FLAGS, 0xFF); err != ErrorCode::Success) return err;

    // Assemble buffer
    std::array<uint8_t, MAX_PAYSIZE + 1> tx{};
    tx[0] = static_cast<uint8_t>(REG_FIFO | 0x80);
    std::copy(buf.begin(), buf.end(), tx.begin()+1);

    // Set TX mode
    if (const auto err = updateRegister(REG_OPMODE, MASK_MODE, MODE_TX); err != ErrorCode::Success) return err;

    // Set Payload
    if (const auto err = writeRegister(REG_PAYLEN, tx.size()-1); err != ErrorCode::Success) return err;
    // Transmit
    if (const auto err = m_interface.spiTransfer(tx, {}); err != ErrorCode::Success) return err;

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