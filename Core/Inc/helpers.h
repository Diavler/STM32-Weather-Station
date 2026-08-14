//
// Created by Haydn on 8/14/2026.
//

#ifndef WEATHERSTATION_HELPERS_H
#define WEATHERSTATION_HELPERS_H
#include <cstdint>

/// Generic error codes
enum class ErrorCode : uint8_t {
    Success,
    Failure,
    ErrorTx,
    ErrorRx
};

/// Convert generic error codes to strings
constexpr const char8_t* errorString(const ErrorCode error) {
    switch (error) {
        case ErrorCode::Success:        return u8"OK\r\n";
        case ErrorCode::Failure:        return u8"ERROR\r\n";
        case ErrorCode::ErrorTx:        return u8"ERROR: TX\r\n";
        case ErrorCode::ErrorRx:        return u8"ERROR: RX\r\n";
        default:                        return u8"ERROR: UNKNOWN\r\n";
    }
}

#endif //WEATHERSTATION_HELPERS_H
