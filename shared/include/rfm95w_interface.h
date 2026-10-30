//
// Created by haydn on 10/29/26.
//

#ifndef WEATHERSTATION_RFM95W_INTERFACE_H
#define WEATHERSTATION_RFM95W_INTERFACE_H
#include <cstdint>
#include <span>

#include "helpers.h"

/// The ESP32 and STM32 implement these functions

class RFM95WInterface
{
    public:
        RFM95WInterface(const RFM95WInterface&) = delete;
        RFM95WInterface& operator=(const RFM95WInterface&) = delete;
        virtual ErrorCode spiTransfer(std::span<const uint8_t> tx,
                                 std::span<uint8_t> rx);
        virtual void reset() = 0;
        virtual void resetCS() = 0;
        virtual void setCS() = 0;



protected:
    ~RFM95WInterface() = default;

};

#endif //WEATHERSTATION_RFM95W_INTERFACE_H
