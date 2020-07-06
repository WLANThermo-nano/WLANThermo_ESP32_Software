/*************************************************** 
    Copyright (C) 2016  Steffen Ochs
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

#include "TemperatureMcp3208.h"
#include <SPI.h>

union SplitTwoBytes {
  uint16_t value;
  struct
  {
    uint8_t lowByte;
    uint8_t highByte;
  };
};

TemperatureMcp3208::TemperatureMcp3208()
{
}

TemperatureMcp3208::TemperatureMcp3208(uint8_t index, uint8_t csPin) : TemperatureBase()
{
  this->address = TEMPERATURE_ADDRESS_INTERNAL;
  this->localIndex = index;
  this->csPin = csPin;
}

void TemperatureMcp3208::update()
{
  if (this->calcTemperature != NULL)
  {
    this->medianValue->AddValue(this->calcTemperature(this->readChip(), this->type));
  }
}

uint16_t TemperatureMcp3208::readChip()
{
  SplitTwoBytes receive;
  SplitTwoBytes command;
  command.value = 0x0400;

  // set channel
  command.value |= ((this->localIndex + 8u) << 6u);

  // write CS
  digitalWrite(csPin, LOW);

  // send first byte
  SPI.transfer(command.highByte);
  // send second byte and receive first 4 bits
  receive.highByte = SPI.transfer(command.lowByte) & 0x0Fu;
  // receive last 8 bits
  receive.lowByte = SPI.transfer(0x00u);

  // write CS
  digitalWrite(csPin, HIGH);

  return receive.value;
}
