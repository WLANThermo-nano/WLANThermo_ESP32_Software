/*************************************************** 
    Copyright (C) 2019  Martin Koerner
    Copyright (C) 2016  Steffen Ochs

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

#include "Notification.h"
#include "Cloud.h"
#include "Settings.h"
#include "temperature/TemperatureGrp.h"

Notification::Notification()
{
  this->loadDefaultValues();
}

void Notification::loadDefaultValues()
{
  memset(&pushTelegram, 0, sizeof(pushTelegram));
  memset(&pushPushover, 0, sizeof(pushPushover));
  memset(&pushApp, 0, sizeof(pushApp));
  memset(&notificationData, 0u, sizeof(Notification));
}

void Notification::check(TemperatureBase *temperature)
{
  if (NULL == temperature)
    return;

  AlarmStatus alarmStatus = temperature->getAlarmStatus();
  AlarmSetting alarmSetting = temperature->getAlarmSetting();
  uint8_t index = TemperatureGrp::getIndex(temperature);

  if (alarmStatus != NoAlarm)
  {
    // handle notification
    if (((AlarmViaPush == alarmSetting) || (AlarmAll == alarmSetting)) && (temperature->getNotificationCounter() > 0u))
    {
      switch (alarmStatus)
      {
      case MinAlarm:
        // add lower limit
        this->notificationData.limit &= ~(1 << index);
        break;
      case MaxAlarm:
        // add upper limit
        this->notificationData.limit |= 1 << index;
        break;
      }
      // Add channel to index
      this->notificationData.index |= 1 << index;
    }
  }
  else
  {
    // delete channel from index
    this->notificationData.index &= ~(1 << index);
    temperature->setNotificationCounter(1u);
  }
}

void Notification::update()
{

  /*if ((this->notificationData.type == 1 && this->pushService.on == 2) || (this->notificationData.type == 2 && this->pushService.on == 1))
  {
    // Testnachricht
    Cloud::sendAPI(APINOTE, NOTELINK);
  }
  else */
  if (this->notificationData.index > 0u)
  { // CHANNEL NOTIFICATION

    for (uint8_t i = 0u; i < 32u; i++)
    {
      if (this->notificationData.index & (1u << i))
      {
        if (this->pushTelegram.enabled || this->pushPushover.enabled || this->pushApp.enabled)
        {
          this->notificationData.channel = i;
          this->notificationData.type = (this->notificationData.index & (1u << i)) ? NotificationType::UpperLimit : NotificationType::LowerLimit;
          Cloud::sendAPI(APINOTIFICATION, NOTELINK);
          break;
        }
      }
    }
  }
}

void Notification::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();

  JsonObject &telegram = json.createNestedObject("telegram");

  telegram["enabled"] = pushTelegram.enabled;
  telegram["token"] = pushTelegram.token;
  telegram["chat_id"] = pushTelegram.chatId;

  JsonObject &pushover = json.createNestedObject("pushover");

  pushover["enabled"] = pushPushover.enabled;
  pushover["token"] = pushPushover.token;
  pushover["user_key"] = pushPushover.userKey;
  pushover["priority"] = pushPushover.priority;

  JsonObject &app = json.createNestedObject("app");

  app["enabled"] = pushApp.enabled;

  JsonArray &devices = app.createNestedArray("devices");

  for (uint8_t i = 0u; i < PUSH_APP_MAX_DEVICES; i++)
  {
    if (strlen(pushApp.devices[i].token) > 0u)
    {
      JsonObject &device = devices.createNestedObject();

      device["name"] = pushApp.devices[i].name;
      device["id"] = pushApp.devices[i].id;
      device["token"] = pushApp.devices[i].token;
    }
  }

  Settings::write(kPush, json);
}

void Notification::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kPush, &jsonBuffer);

  if (json.success())
  {
    // migrate from old structure
    if (json.containsKey("onP") && json.containsKey("tokP") &&
        json.containsKey("idP") && json.containsKey("rptP") &&
        json.containsKey("svcP"))
    {
      switch (json["svcP"].as<uint8_t>())
      {
      // telegram
      case 0u:
        pushTelegram.enabled = json["onP"];
        strcpy(pushTelegram.token, json["tokP"].asString());
        pushTelegram.chatId = json["idP"];
        break;
      // pushover
      case 1u:
        pushPushover.enabled = json["onP"];
        strcpy(pushPushover.token, json["tokP"].asString());
        strcpy(pushPushover.userKey, json["idP"].asString());
      }
    }
  }
}
