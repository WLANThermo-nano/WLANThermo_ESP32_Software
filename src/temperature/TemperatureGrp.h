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

#include "Arduino.h"
#include "TemperatureBase.h"

#define MAX_TEMPERATURES 20u

class TemperatureGrp
{
public:
  TemperatureGrp();
  void virtual update();
  void add(TemperatureBase *temperature);
  void addBle();
  void removeBle();
  TemperatureBase *operator[](int index);
  uint8_t count();
  boolean setUnit(TemperatureUnit unit);
  TemperatureUnit getUnit();
  TemperatureBase *getNextActive(uint8_t index);
  uint32_t getActiveBits();
  uint8_t getActiveCount();
  boolean hasAlarm();
  void acknowledgeAlarm();
  void saveConfig();
  void loadConfig();

private:
  TemperatureBase *addRemote(uint8_t type, const char *address, uint8_t localIndex);
  TemperatureBase *temperatures[MAX_TEMPERATURES];
  uint8_t addIndex;
  TemperatureUnit currentUnit;
};