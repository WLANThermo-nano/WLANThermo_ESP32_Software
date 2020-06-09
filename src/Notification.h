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
#include "temperature/TemperatureBase.h"

// NOTIFICATION
struct NotificationData
{
  uint32_t index; // INDEX BIN
  uint8_t ch;     // CHANNEL BIN
  uint32_t limit; // LIMIT: 0 = LOW TEMPERATURE, 1 = HIGH TEMPERATURE
  byte type;      // TYPE: 0 = NORMAL MODE, 1 = TEST MESSAGE
};

struct PushService
{
  byte on;        // NOTIFICATION SERVICE OFF(0)/ON(1)/TEST(2)/CLEAR(3)
  String token;   // API TOKEN
  String id;      // CHAT ID
  uint8_t repeat; // REPEAT PUSH NOTIFICATION
  byte service;   // SERVICE
};

class Notification
{
public:
  Notification();
  void saveConfig();
  void loadConfig();
  PushService getConfig();
  void setConfig(PushService newConfig);
  void loadDefaultValues();
  void check(TemperatureBase *temperature);
  void update();
  PushService pushService;
  NotificationData notificationData;

private:
};
