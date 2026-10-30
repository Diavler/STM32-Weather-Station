//
// Created by haydn on 10/30/26.
//
#include "rfm95w_stm32interface.h"

ErrorCode
RFM95WInterfaceSTM32::spiTransfer(const std::span<const uint8_t> tx, std::span<uint8_t> rx)
{
    resetCS();
    const auto halStatus = rx.empty()
        ? HAL_SPI_Transmit(m_hspiPtr, tx.data(), tx.size(), HAL_MAX_DELAY)
        : HAL_SPI_TransmitReceive(m_hspiPtr, tx.data(), rx.data(), tx.size(), HAL_MAX_DELAY);
    setCS();

    return (halStatus == HAL_OK) ? ErrorCode::Success : ErrorCode::spiError;
}

void RFM95WInterfaceSTM32::reset() {
    HAL_GPIO_WritePin(m_rstPort, m_rstPin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(m_rstPort, m_rstPin, GPIO_PIN_SET);
    HAL_Delay(10);
}