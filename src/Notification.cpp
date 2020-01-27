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

Notification::Notification()
{
  this->loadDefaultValues();
}

void Notification::loadDefaultValues()
{
  pushService.on = 0;
  pushService.token = "";
  pushService.id = "";
  pushService.repeat = 1;
  pushService.service = 0;

  memset(&notificationData, 0u, sizeof(Notification));
}

void Notification::check(TemperatureBase *temperature)
{
  if (NULL == temperature)
    return;

  AlarmStatus alarmStatus = temperature->getAlarmStatus();
  AlarmSetting alarmSetting = temperature->getAlarmSetting();
  uint8_t index = temperature->getGlobalIndex();

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
    temperature->setNotificationCounter(this->pushService.repeat);
  }
}

void Notification::update()
{

  if ((this->notificationData.type == 1 && this->pushService.on == 2) || (this->notificationData.type == 2 && this->pushService.on == 1))
  { // Testnachricht
    if (Cloud::sendAPI(0))
    {
      Cloud::apiindex = APINOTE;
      Cloud::urlindex = NOTELINK;
      Cloud::parindex = NOPARA;
      Cloud::sendAPI(2);
    }
  }
  else if (this->notificationData.index > 0)
  { // CHANNEL NOTIFICATION

    for (int i = 0; i < 32u; i++)
    {
      if (this->notificationData.index & (1 << i))
      {
        if (this->pushService.on > 0)
        {
          if (Cloud::sendAPI(0))
          {
            this->notificationData.ch = i;
            Cloud::apiindex = APINOTE;
            Cloud::urlindex = NOTELINK;
            Cloud::parindex = NOPARA;
            Cloud::sendAPI(2); // Notification per Nano-Server
          }
        }
      }
    }
  }

  if (this->pushService.on == 3)
    loadConfig();
}

void Notification::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();

  json["onP"] = pushService.on;
  json["tokP"] = pushService.token;
  json["idP"] = pushService.id;
  json["rptP"] = pushService.repeat;
  json["svcP"] = pushService.service;

  Settings::write(kPush, json);
}

void Notification::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kPush, &jsonBuffer);

  if (json.success())
  {

    if (json.containsKey("onP"))
      pushService.on = json["onP"];
    if (json.containsKey("tokP"))
      pushService.token = json["tokP"].asString();
    if (json.containsKey("idP"))
      pushService.id = json["idP"].asString();
    if (json.containsKey("rptP"))
      pushService.repeat = json["rptP"];
    if (json.containsKey("svcP"))
      pushService.service = json["svcP"];
  }
}

PushService Notification::getConfig()
{
  return pushService;
}

void Notification::setConfig(PushService newConfig)
{
  // copy new config
  pushService = newConfig;

  // save to NvM
  saveConfig();
}