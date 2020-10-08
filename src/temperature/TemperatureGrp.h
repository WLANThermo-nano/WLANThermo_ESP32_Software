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

typedef void (*TemperatureCallback_t)(uint8_t index, class TemperatureBase *, boolean, void *);

typedef struct TemperatureCallbackData
{
  TemperatureCallback_t cb;
  void *userData;
} TemperatureCallbackDataType;

class TemperatureGrp
{
public:
  TemperatureGrp();
  void update();
  void refresh();
  void add(TemperatureBase *temperature);
  void add(uint8_t type, String address, uint8_t localIndex);
  void remove(uint8_t type, String address, uint8_t localIndex);
  void remove(uint8_t index);
  boolean exists(uint8_t type, String address, uint8_t localIndex);
  TemperatureBase *operator[](int index);
  static uint8_t count();
  boolean setUnit(TemperatureUnit unit);
  TemperatureUnit getUnit();
  TemperatureBase *getNextActive(uint8_t index);
  uint32_t getActiveBits();
  uint8_t getActiveCount();
  boolean hasAlarm(boolean filterAcknowledged = true);
  void acknowledgeAlarm();
  void saveConfig();
  void loadConfig();
  void registerCallback(TemperatureCallback_t callback, void *userData = NULL);

  static uint8_t getIndex(TemperatureBase *temperature);

private:
  static std::vector<TemperatureBase *> temperatures;
  std::vector<TemperatureCallbackDataType> registeredCb;
  TemperatureUnit currentUnit;
};