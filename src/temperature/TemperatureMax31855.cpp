/*************************************************** 
    Copyright (C) 2019  Martin Koerner

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
    HISTORY: Please refer Github History
    
****************************************************/

#include <SPI.h>
#include "TemperatureMax31855.h"
#include "ArduinoLog.h"

#define MAX31855_FAULT_BITS 0x07u
#define MAX31855_TEMPERATURE_MASK 0x1FFFu
#define MAX31855_TEMPERATURE_SHIFT 18u
#define MAX31855_BUILT_IN_TEST_SHIFT 4u
#define MAX31855_NEGATIVE_SIGN_BIT 0x2000u
#define MAX31855_TEMPERATURE_UNIT 0.25

union SplitFourBytes
{
  uint32_t value;
  struct
  {
    uint8_t byte0;
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
  };
};

TemperatureMax31855::TemperatureMax31855()
{
}

TemperatureMax31855::TemperatureMax31855(uint8_t index, uint8_t csPin) : TemperatureBase()
{
  this->address = TEMPERATURE_ADDRESS_TYPE_K;
  this->localIndex = index;
  this->csPin = csPin;
  this->fixedSensor = true;
  this->type = SensorType::TypeK;
}

void TemperatureMax31855::update()
{
  this->medianValue->AddValue(this->calcTemperatureTypeK(this->readChip()));
}

boolean TemperatureMax31855::isBuiltIn()
{
  boolean chipAvailable = false;
  uint32_t value = this->readChip();

  // if any of the fault bits is set then the chip is builtin
  if (value & MAX31855_FAULT_BITS)
  {
    chipAvailable = true;
  }
  // if any of bit 31 to bit 5 is non zero then the chip is builtin
  else if (value >> MAX31855_BUILT_IN_TEST_SHIFT)
  {
    chipAvailable = true;
  }

  if (chipAvailable)
  {
    Log.notice("Type K has been detected" CR);
    Log.trace("Type K readChip: %X" CR, value);
  }

  return chipAvailable;
}

uint32_t TemperatureMax31855::readChip()
{
  SplitFourBytes receive;

  // write CS
  digitalWrite(csPin, LOW);
  delay(1u);

  receive.byte3 = SPI.transfer(0u);
  receive.byte2 = SPI.transfer(0u);
  receive.byte1 = SPI.transfer(0u);
  receive.byte0 = SPI.transfer(0u);

  // write CS
  digitalWrite(csPin, HIGH);

  return receive.value;
}

float TemperatureMax31855::calcTemperatureTypeK(uint32_t rawValue)
{
  float temperature = INACTIVEVALUE;
  uint32_t calcValue = rawValue;

  // Check error bits
  if (calcValue & MAX31855_FAULT_BITS)
  {
    // Do nothing
  }
  else
  {
    calcValue = calcValue >> MAX31855_TEMPERATURE_SHIFT;

    // handle negative value
    if (calcValue & MAX31855_NEGATIVE_SIGN_BIT)
    {
      calcValue = ~calcValue;
      temperature = calcValue & MAX31855_TEMPERATURE_MASK;
      temperature += 1;
      temperature *= -1;
    }
    // handle positive value
    else
    {
      temperature = calcValue & MAX31855_TEMPERATURE_MASK;
    }

    temperature *= MAX31855_TEMPERATURE_UNIT;
  }

  return temperature;
}
