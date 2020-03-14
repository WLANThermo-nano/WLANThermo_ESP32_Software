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

#include "TemperatureMax11615.h"

#define MAX1161X_SGL_DIF_BIT 0x01u
#define MAX1161X_SCAN0_BIT 0x20u
#define MAX1161X_SET_CSX(index) (index << 0x01u)

union SplitTwoBytes {
  uint16_t value;
  struct
  {
    uint8_t lowByte;
    uint8_t highByte;
  };
};

TemperatureMax11615::TemperatureMax11615()
{
}

TemperatureMax11615::TemperatureMax11615(uint8_t index, TwoWire *twoWire) : TemperatureBase()
{
  this->localIndex = index;
  this->twoWire = twoWire;

  // MAX11615
  byte reg = 0xA0; // A0 = 10100000
  // page 14
  // 1: setup mode
  // SEL:010 = Reference (Table 6)
  // external(1)/internal(0) clock
  // unipolar(0)/bipolar(1)
  // 0: reset the configuration register to default
  // 0: dont't care

  this->twoWire->beginTransmission(this->chipAddress);
  this->twoWire->write(reg);
  this->twoWire->endTransmission();
}

void TemperatureMax11615::update()
{
  if (this->calcTemperature != NULL)
  {
    this->currentValue = this->calcTemperature(this->readChip(), this->type);
  }
}

uint16_t TemperatureMax11615::readChip()
{
  byte config;
  SplitTwoBytes receive;

  config = MAX1161X_SGL_DIF_BIT | MAX1161X_SCAN0_BIT | MAX1161X_SET_CSX(this->localIndex);

  this->twoWire->beginTransmission(this->chipAddress);
  this->twoWire->write(config);
  this->twoWire->endTransmission();

  this->twoWire->requestFrom(this->chipAddress, sizeof(receive));

  receive.highByte = this->twoWire->read();
  receive.lowByte = this->twoWire->read();

  return receive.value & 4095;
}
