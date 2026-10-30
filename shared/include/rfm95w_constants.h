//
// Created by haydn on 10/29/26.
//

#ifndef WEATHERSTATION_RFM95W_CONSTANTS_H
#define WEATHERSTATION_RFM95W_CONSTANTS_H
#include <cstdint>

/* Start of Constants */

static constexpr uint8_t REG_OPMODE     = 0x01;
static constexpr uint8_t MASK_MODE      = 0b111;
static constexpr uint8_t MODE_SLEEP     = 0b000;
static constexpr uint8_t MODE_STANDBY   = 0b001;
static constexpr uint8_t MODE_TX        = 0b011;
static constexpr uint8_t MODE_RXC       = 0b101;
static constexpr uint8_t MODE_RXS       = 0b110;
static constexpr uint8_t MODE_CAD       = 0b111;
static constexpr uint8_t MODE_LORA      = 0x80;

// 915Mhz compensation
static constexpr uint8_t REG_FR_MSB     = 0x06;
static constexpr uint8_t REG_FR_MID     = 0x07;
static constexpr uint8_t REG_FR_LSB     = 0x08;
static constexpr uint8_t VAL_FR_MSB     = 0b11100100;
static constexpr uint8_t VAL_FR_MID     = 0b11000000;
static constexpr uint8_t VAL_FR_LSB     = 0b00000000;

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

/* End of Constants */

#endif //WEATHERSTATION_RFM95W_CONSTANTS_H
