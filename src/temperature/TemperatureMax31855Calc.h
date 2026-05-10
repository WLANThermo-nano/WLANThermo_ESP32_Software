#pragma once
#include <stdint.h>

// Returned when fault bits are set (open-circuit, short to VCC/GND).
// Matches INACTIVEVALUE from TemperatureBase.h.
static const float MAX31855_INACTIVE_VALUE = 999.0f;

// Pure bit-manipulation conversion of a MAX31855 32-bit word to °C.
// No SPI, no Arduino types — directly testable on any host.
inline float calcMax31855Temperature(uint32_t rawValue)
{
    static const uint32_t FAULT_BITS   = 0x07u;
    static const uint32_t TEMP_MASK    = 0x1FFFu;
    static const uint32_t TEMP_SHIFT   = 18u;
    static const uint32_t NEG_SIGN_BIT = 0x2000u;
    static const float    TEMP_UNIT    = 0.25f;
    static const float    TEMP_ADJ     = 1.0f;

    if (rawValue & FAULT_BITS) {
        return MAX31855_INACTIVE_VALUE;
    }

    uint32_t calcValue  = rawValue >> TEMP_SHIFT;
    float    temperature;

    if (calcValue & NEG_SIGN_BIT) {
        calcValue   = ~calcValue;
        temperature = (float)(calcValue & TEMP_MASK) + 1.0f;
        temperature = -temperature;
    } else {
        temperature = (float)(calcValue & TEMP_MASK);
    }

    temperature *= TEMP_UNIT;
    temperature -= TEMP_ADJ;
    return temperature;
}
