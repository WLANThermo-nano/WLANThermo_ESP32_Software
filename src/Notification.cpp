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
#include "mbedtls/md.h"

#define PUSHOVER_RETRY_DEFAULT 30u
#define PUSHOVER_EXPIRE_DEFAULT 300u

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

  pushPushover.retry = PUSHOVER_RETRY_DEFAULT;
  pushPushover.expire = PUSHOVER_EXPIRE_DEFAULT;
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
        this->notificationData.limit &= ~(1u << index);
        break;
      case MaxAlarm:
        // add upper limit
        this->notificationData.limit |= 1u << index;
        break;
      }
      // Add channel to index
      this->notificationData.index |= 1u << index;

      temperature->updateNotificationCounter();
    }
  }
  else
  {
    // delete channel from index
    this->notificationData.index &= ~(1 << index);
    temperature->setNotificationCounter(1u);
  }
}

NotificationService getService(String service)
{
  NotificationService notificationService = NotificationService::None;

  if (service == "telegram")
  {
    notificationService = NotificationService::Telegram;
  }
  else if (service == "pushover")
  {
    notificationService = NotificationService::Pushover;
  }
  else if (service == "app")
  {
    notificationService = NotificationService::App;
  }

  return notificationService;
}

void Notification::setTelegramConfig(PushTelegramType config, boolean testMessage)
{
  if (true == testMessage)
  {
    sendTestMessage(NotificationService::Telegram, &config);
  }
  else
  {
    pushTelegram = config;
  }
}
void Notification::setPushoverConfig(PushPushoverType config, boolean testMessage)
{
  // load default when values not set
  if ((0u == config.expire) || (0u == config.retry))
  {
    config.retry = PUSHOVER_RETRY_DEFAULT;
    config.expire = PUSHOVER_EXPIRE_DEFAULT;
  }

  if (true == testMessage)
  {
    sendTestMessage(NotificationService::Pushover, &config);
  }
  else
  {
    pushPushover = config;
  }
}
void Notification::setAppConfig(PushAppType config, boolean testMessage)
{
  if (true == testMessage)
  {
    sendTestMessage(NotificationService::App, &config);
  }
  else
  {
    pushApp = config;
  }
}

void Notification::sendTestMessage(NotificationService service, void *config)
{
  notificationData.testService = service;
  notificationData.testConfig = config;
  notificationData.type = NotificationType::Test;

  if (NotificationService::None != notificationData.testService)
  {
    Cloud::sendAPI(APINOTIFICATION, NOTELINK);
  }

  notificationData.testService = NotificationService::None;
  notificationData.testConfig = NULL;
}

void Notification::update()
{
  if (this->notificationData.index > 0u)
  { // CHANNEL NOTIFICATION

    for (uint8_t i = 0u; i < 32u; i++)
    {
      if (this->notificationData.index & (1u << i))
      {
        if (this->pushTelegram.enabled || this->pushPushover.enabled || this->pushApp.enabled)
        {
          this->notificationData.channel = i;
          this->notificationData.type = (this->notificationData.limit & (1u << i)) ? NotificationType::UpperLimit : NotificationType::LowerLimit;
          Cloud::sendAPI(APINOTIFICATION, NOTELINK);
          this->notificationData.index &= ~(1u << i);
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
  pushover["retry"] = pushPushover.retry;
  pushover["expire"] = pushPushover.expire;

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
    // load current structure
    else
    {
      if (json.containsKey("telegram"))
      {
        JsonObject &_telegram = json["telegram"];

        if (_telegram.containsKey("enabled") && _telegram.containsKey("token") &&
            _telegram.containsKey("chat_id"))
        {
          // load telegram
          pushTelegram.enabled = _telegram["enabled"];
          strcpy(pushTelegram.token, _telegram["token"].asString());
          pushTelegram.chatId = _telegram["chat_id"];
        }
      }

      if (json.containsKey("pushover"))
      {
        JsonObject &_pushover = json["pushover"];

        if (_pushover.containsKey("enabled") && _pushover.containsKey("token") &&
            _pushover.containsKey("user_key") && _pushover.containsKey("priority") &&
            _pushover.containsKey("retry") && _pushover.containsKey("expire"))
        {
          // load pushover
          pushPushover.enabled = _pushover["enabled"];
          strcpy(pushPushover.token, _pushover["token"].asString());
          strcpy(pushPushover.userKey, _pushover["user_key"].asString());
          pushPushover.retry = _pushover["retry"];
          pushPushover.expire = _pushover["expire"];
        }
      }

      if (json.containsKey("app"))
      {
        JsonObject &_app = json["app"];

        if (_app.containsKey("enabled") && _app.containsKey("devices"))
        {
          // load app
          pushApp.enabled = _app["enabled"];

          JsonArray &_devices = _app["devices"];
          uint8_t deviceIndex = 0u;

          for (JsonArray::iterator it = _devices.begin(); (it != _devices.end()) && (deviceIndex < PUSH_APP_MAX_DEVICES); ++it)
          {
            JsonObject &_device = it->asObject();

            if (_device.containsKey("name") && _device.containsKey("id") &&
                _device.containsKey("token"))
            {
              strcpy(pushApp.devices[deviceIndex].name, _device["name"].asString());
              strcpy(pushApp.devices[deviceIndex].id, _device["id"].asString());
              strcpy(pushApp.devices[deviceIndex].token, _device["token"].asString());
              deviceIndex++;
            }
          }
        }
      }
    }
  }
}

String Notification::getTokenSha256(String token)
{
  byte shaResult[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;

  const size_t tokenLength = strlen(token.c_str());

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *)token.c_str(), tokenLength);
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);

  String hash;
  for (uint8_t index = 0u; index < sizeof(shaResult); index++)
  {
    char byteString[3];
    sprintf(byteString, "%02x", (int)shaResult[index]);
    hash += byteString;
  }

  return hash;
}

String Notification::getDeviceTokenFromHash(String hash)
{
  String token;

  for (uint8_t deviceIndex = 0u; deviceIndex < PUSH_APP_MAX_DEVICES; deviceIndex++)
  {
    if(getTokenSha256(pushApp.devices[deviceIndex].token) == hash)
    {
      token = pushApp.devices[deviceIndex].token;
      break;
    }
  }

  return token;
}
