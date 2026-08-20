//
// Created by Haydn on 8/14/2026.
//
#include "ssd1315.h"

/* Notes
 * D/C# --> I2C acts as SA0
 *
 *
 *
 */

static constexpr uint8_t SLAVE_ADDR         = 0b01111000; // SA0 = 0, LSB = 0


static constexpr uint8_t H_PIXELS           = 128;
static constexpr uint8_t V_PIXELS           = 64;