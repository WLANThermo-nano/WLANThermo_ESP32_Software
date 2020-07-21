/*************************************************** 
    Copyright (C) 2020  Martin Koerner

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

#define NUM_OF_MAV_RADIO_TEMPERATURES 2u

class TemperatureMavRadio : public TemperatureBase
{
public:
  TemperatureMavRadio();
  TemperatureMavRadio(uint8_t index);
  void update();
  void refresh();
  static boolean initReceiver(uint8_t rxPin);

private:
  static int getTemperatureValue(uint8_t index);
  static uint32_t lastUpdateMillis;
  static int temperatureValue[NUM_OF_MAV_RADIO_TEMPERATURES];
};
