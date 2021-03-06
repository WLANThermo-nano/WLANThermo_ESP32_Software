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
#pragma once

#include "TemperatureBase.h"
#include "Wire.h"

#define MAX11615_ADDRESS 0x33u

class TemperatureMax11615 : public TemperatureBase
{
public:
  TemperatureMax11615();
  TemperatureMax11615(uint8_t index, TwoWire *twoWire);
  void update();

private:
  uint16_t readChip();
  TwoWire *twoWire;
  const uint8_t chipAddress = MAX11615_ADDRESS;
};
