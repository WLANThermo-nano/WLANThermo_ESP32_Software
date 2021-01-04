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

#define PUSH_APP_MAX_DEVICES 3u

enum class NotificationType
{
  Test,
  LowerLimit,
  UpperLimit,
  Battery
};

// NOTIFICATION
struct NotificationData
{
  uint32_t index;
  uint32_t limit;
  uint8_t channel;
  NotificationType type;
};

typedef struct
{
  boolean enabled;
  char token[100];
  int chatId;
} PushTelegramType;

typedef struct
{
  boolean enabled;
  char token[31];
  char userKey[31];
  uint8_t priority;
} PushPushoverType;

typedef struct
{
  char name[31];
  char id[65];
  char token[255];
} PushAppDeviceType;

typedef struct
{
  boolean enabled;
  PushAppDeviceType devices[PUSH_APP_MAX_DEVICES];
} PushAppType;

class Notification
{
public:
  Notification();
  void saveConfig();
  void loadConfig();
  void loadDefaultValues();
  PushTelegramType getTelegramConfig() { return pushTelegram; };
  void setTelegramConfig(PushTelegramType config) { pushTelegram = config; };
  PushPushoverType getPushoverConfig() { return pushPushover; };
  void setPushoverConfig(PushPushoverType config) { pushPushover = config; };
  PushAppType getAppConfig() { return pushApp; };
  void setAppConfig(PushAppType config) { pushApp = config; };
  void check(TemperatureBase *temperature);
  void update();
  NotificationData notificationData;

private:
  PushTelegramType pushTelegram;
  PushPushoverType pushPushover;
  PushAppType pushApp;
};
