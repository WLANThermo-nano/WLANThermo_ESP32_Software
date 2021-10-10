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
  Test = 0,
  LowerLimit,
  UpperLimit,
  Battery
};

enum class NotificationService
{
  None,
  Telegram,
  Pushover,
  App
};

// NOTIFICATION
typedef struct
{
  uint32_t index;
  uint32_t limit;
  uint8_t channel;
  NotificationType type;
  NotificationService testService;
  void *testConfig;
} NotificationData;

typedef struct
{
  boolean enabled;
  char token[100];
  char chatId[32];
} PushTelegramType;

typedef struct
{
  boolean enabled;
  char token[31];
  char userKey[31];
  uint8_t priority;
  uint32_t retry;
  uint32_t expire;
} PushPushoverType;

typedef struct
{
  char name[31];
  char id[65];
  char token[255];
  uint8_t sound;
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
  PushPushoverType getPushoverConfig() { return pushPushover; };
  PushAppType getAppConfig() { return pushApp; };
  NotificationData getNotificationData() { return notificationData; };
  void setTelegramConfig(PushTelegramType config, boolean testMessage);
  void setPushoverConfig(PushPushoverType config, boolean testMessage);
  void setAppConfig(PushAppType config, boolean testMessage);
  void sendTestMessage(NotificationService service, void *config);
  static String getTokenSha256(String token);
  String getDeviceTokenFromHash(String hash);
  String getNotificationSound(uint8_t soundIndex);
  void check(TemperatureBase *temperature);
  void update();

private:
  NotificationData notificationData;
  PushTelegramType pushTelegram;
  PushPushoverType pushPushover;
  PushAppType pushApp;
  
};
