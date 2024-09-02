/*************************************************** 
    Copyright (C) 2016  Steffen Ochs
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
#include "API.h"
#include "system/SystemBase.h"
#include "display/DisplayBase.h"
#include "DeviceId.h"
#include "Version.h"
#include "WebHandler.h"
#include "DbgPrint.h"
#include "RecoveryMode.h"

API::API()
{
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Display JSON Object - Send everytime when connect to API
void API::displayObj(JsonObject &jObj)
{
  jObj["updname"] = gDisplay->getUpdateName();
  jObj["orientation"] = (uint16_t)gDisplay->getOrientation();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Device JSON Object - Send everytime when connect to API
void API::deviceObj(JsonObject &jObj)
{

  jObj["device"] = gSystem->getDeviceName();
  jObj["serial"] = gSystem->getSerialNumber();
  jObj["cpu"] = gSystem->getCpuName();
  jObj["flash_size"] = gSystem->getFlashSize();

  if (gSystem->item.read(ItemNvsKeys::kItem) != "")
    jObj["item"] = gSystem->item.read(ItemNvsKeys::kItem);

  jObj["hw_version"] = String("v") + String(gSystem->getHardwareVersion());

  jObj["sw_version"] = FIRMWAREVERSION;
  jObj["api_version"] = SERVERAPIVERSION;
  jObj["language"] = gSystem->getLanguage();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// System JSON Object
void API::systemObj(JsonObject &jObj, bool settings)
{

  jObj["time"] = String(now());
  jObj["unit"] = String((char)gSystem->temperatures.getUnit());

  if (!settings)
  {
    if (gSystem->battery)
    {
      jObj["soc"] = gSystem->battery->percentage;
      jObj["charge"] = gSystem->battery->isCharging();
    }
    jObj["rssi"] = gSystem->wlan.getRssi();
    jObj["online"] = gSystem->cloud.state;
  }
  else
  {
    jObj["ap"] = gSystem->wlan.getAccessPointName();
    jObj["host"] = gSystem->wlan.getHostName();
    jObj["language"] = gSystem->getLanguage();
    jObj["version"] = FIRMWAREVERSION;
    jObj["getupdate"] = gSystem->otaUpdate.getVersion();
    jObj["autoupd"] = gSystem->otaUpdate.getAutoUpdate();
    jObj["prerelease"] = gSystem->otaUpdate.getPrerelease();
    jObj["crashreport"] = gSystem->getCrashReport();
    jObj["hwversion"] = String("V") + String(gSystem->getHardwareVersion());
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Channel JSON Array
void API::channelAry(JsonArray &jAry, int cc)
{

  int i = 0;
  if (cc < gSystem->temperatures.count())
    i = cc - 1; // nur ein Channel

  for (i; i < cc; i++)
  {
    TemperatureBase *temperature = gSystem->temperatures[i];
    if (temperature != NULL)
    {
      JsonObject &data = jAry.createNestedObject();
      data["number"] = i + 1;
      data["name"] = temperature->getName();
      data["typ"] = temperature->getType();
      data["temp"] = limit_float(temperature->getValue(), i);
      data["min"] = temperature->getMinValue();
      data["max"] = temperature->getMaxValue();
      data["alarm"] = (uint8_t)temperature->getAlarmSetting();
      data["color"] = temperature->getColor();
      data["fixed"] = temperature->isFixedSensor();
      data["connected"] = temperature->isConnected();
    }
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Pitmaster Types JSON Array
void API::pitTyp(JsonObject &jObj)
{

  JsonArray &_typ = jObj.createNestedArray("type");
  _typ.add("off");
  _typ.add("manual");
  _typ.add("auto");
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Pitmaster JSON Array
void API::pitAry(JsonArray &jAry, int cc)
{

  String sc[2] = {"#ff0000", "#FE2EF7"};
  String vc[2] = {"#000000", "#848484"};

  for (uint8_t i = 0u; i < cc; i++)
  {
    Pitmaster *pm = gSystem->pitmasters[i];
    if (pm)
    {
      JsonObject &ma = jAry.createNestedObject();
      ma["id"] = i;
      ma["channel"] = TemperatureGrp::getIndex(pm->getAssignedTemperature()) + 1u;
      ma["pid"] = pm->getAssignedProfile()->id;
      ma["value"] = (uint8_t)pm->getValue();
      ma["set"] = pm->getTargetTemperature();
      switch (pm->getType())
      {
      case pm_off:
        ma["typ"] = "off";
        break;
      case pm_manual:
        ma["typ"] = "manual";
        break;
      case pm_auto:
        ma["typ"] = "auto";
        break;
      }
      switch (pm->getTypeLast())
      {
      case pm_manual:
        ma["typ_last"] = "manual";
        break;
      case pm_auto:
        ma["typ_last"] = "auto";
        break;
      }
      ma["set_color"] = sc[i];
      ma["value_color"] = vc[i];
    }
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PID JSON Array
void API::pidAry(JsonArray &jAry, int cc)
{
  for (int i = 0; i < gSystem->getPitmasterProfileCount(); i++)
  { // pidsize
    PitmasterProfile *profile = gSystem->getPitmasterProfile(i);

    JsonObject &_pid = jAry.createNestedObject();
    _pid["name"] = profile->name;
    _pid["id"] = profile->id;
    _pid["aktor"] = profile->actuator;
    _pid["Kp"] = limit_float(profile->kp, -1);
    _pid["Ki"] = limit_float(profile->ki, -1);
    _pid["Kd"] = limit_float(profile->kd, -1);
    _pid["DCmmin"] = profile->dcmin;
    _pid["DCmmax"] = profile->dcmax;
    _pid["opl"] = profile->opl;
    _pid["SPmin"] = profile->spmin;
    _pid["SPmax"] = profile->spmax;
    _pid["link"] = profile->link;
    _pid["tune"] = profile->autotune; // noch nicht im EE gespeichert
    _pid["jp"] = profile->jumppw;
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// IoT JSON Object
void API::iotObj(JsonObject &jObj)
{
  MqttConfig mqttConfig = gSystem->mqtt.getConfig();
  CloudConfig cloudConfig = gSystem->cloud.getConfig();
  jObj["PMQhost"] = String(mqttConfig.host);
  jObj["PMQport"] = mqttConfig.port;
  jObj["PMQuser"] = String(mqttConfig.user);
  jObj["PMQpass"] = String(mqttConfig.password);
  jObj["PMQqos"] = mqttConfig.QoS;
  jObj["PMQon"] = mqttConfig.enabled;
  jObj["PMQint"] = mqttConfig.interval;
  jObj["CLon"] = cloudConfig.cloudEnabled;
  jObj["CLtoken"] = cloudConfig.cloudToken;
  jObj["CLint"] = cloudConfig.cloudInterval;
  jObj["CLurl"] = "cloud.wlanthermo.de/index.html";

  jObj["CCLon"] = cloudConfig.customEnabled;
  jObj["CCLint"] = cloudConfig.customInterval;
  jObj["CCLurl"] = cloudConfig.customUrl;
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Notification JSON Object
void API::notificationObj(JsonObject &jObj)
{
  PushTelegramType pushTelegram = gSystem->notification.getTelegramConfig();
  PushPushoverType pushPushover = gSystem->notification.getPushoverConfig();
  PushAppType pushApp = gSystem->notification.getAppConfig();
  NotificationData notificationData = gSystem->notification.getNotificationData();

  JsonObject &_message = jObj.createNestedObject("message");
  JsonArray &_services = jObj.createNestedArray("services");

  _message["type"] = (uint32_t)notificationData.type;

  // Handle test message
  if (NotificationType::Test == notificationData.type)
  {
    // Disable all services
    pushTelegram.enabled = false;
    pushPushover.enabled = false;
    pushApp.enabled = false;

    // Copy test config and enable test service
    switch (notificationData.testService)
    {
    case NotificationService::Telegram:
      memcpy(&pushTelegram, notificationData.testConfig, sizeof(PushTelegramType));
      pushTelegram.enabled = true;
      break;
    case NotificationService::Pushover:
      memcpy(&pushPushover, notificationData.testConfig, sizeof(PushPushoverType));
      pushPushover.enabled = true;
      break;
    case NotificationService::App:
      memcpy(&pushApp, notificationData.testConfig, sizeof(PushAppType));
      pushApp.enabled = true;
      break;
    }
  }
  // Add channel for temperature alarms
  else if ((NotificationType::LowerLimit == notificationData.type) ||
           (NotificationType::UpperLimit == notificationData.type))
  {
    _message["channel"] = (uint32_t)notificationData.channel;
    _message["unit"] = String((char)gSystem->temperatures.getUnit());
    TemperatureBase *temperature = gSystem->temperatures[notificationData.channel];

    if (temperature)
    {
      _message["temp"] = (int)temperature->getValue();
      _message["limit"] = (NotificationType::LowerLimit == notificationData.type) ? temperature->getMinValue() : temperature->getMaxValue();
    }
  }

  if (pushTelegram.enabled)
  {
    JsonObject &_telegram = _services.createNestedObject();
    _telegram["service"] = "telegram";
    _telegram["token"] = String(pushTelegram.token);
    _telegram["chat_id"] = String(pushTelegram.chatId);
  }

  if (pushPushover.enabled)
  {
    JsonObject &_pushover = _services.createNestedObject();
    _pushover["service"] = "pushover";
    _pushover["token"] = String(pushPushover.token);
    _pushover["user_key"] = String(pushPushover.userKey);
    _pushover["priority"] = pushPushover.priority;
    _pushover["retry"] = pushPushover.retry;
    _pushover["expire"] = pushPushover.expire;
  }

  if (pushApp.enabled)
  {
    for (uint8_t i = 0u; i < PUSH_APP_MAX_DEVICES; i++)
    {
      if (strlen(pushApp.devices[i].token) > 0u)
      {
        JsonObject &_app = _services.createNestedObject();
        _app["service"] = "app";
        _app["phone_id"] = String(pushApp.devices[i].id);
        _app["name"] = String(pushApp.devices[i].name);
        _app["token"] = String(pushApp.devices[i].token);
        _app["device_id"] = DeviceId::get();
        _app["sound"] = gSystem->notification.getNotificationSound(pushApp.devices[i].sound);
        _app["android_channel_id"] = String(pushApp.devices[i].androidchannelid);
      }
    }
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Crash JSON Object
void API::crashObj(JsonObject &jObj)
{
  jObj["reset_reason"] = gSystem->getResetReason(0u) + String(";") + gSystem->getResetReason(1u);
  jObj["reset_counter"] = RecoveryMode::getResetCounter();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Update JSON Object
void API::updateObj(JsonObject &jObj)
{
  jObj["prerelease"] = gSystem->otaUpdate.getPrerelease();

  // nach einer bestimmten Version fragen
  if (gSystem->otaUpdate.getRequestedVersion() != "false")
    jObj["version"] = gSystem->otaUpdate.getRequestedVersion();

  if (gSystem->otaUpdate.getRequestedFile() != "")
  {
    jObj["file"] = gSystem->otaUpdate.getRequestedFile();
    // include also preleases for file request
    jObj["prerelease"] = true;
  }

  if (gSystem->otaUpdate.getForceFlag())
    jObj["force"] = gSystem->otaUpdate.getForceFlag();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// URL JSON Object
void API::urlObj(JsonObject &jObj)
{

  /*
  for (int i = 0; i < NUMITEMS(serverurl); i++) {
  
    JsonObject& _obj = jObj.createNestedObject(serverurl[i].typ);
    _obj["host"] =  serverurl[i].host;
    _obj["page"] =  serverurl[i].page;
  }
*/
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// DATA JSON Object
void API::dataObj(JsonObject &jObj, bool cloud)
{
  // SYSTEM
  JsonObject &_system = jObj.createNestedObject("system");
  systemObj(_system);

  // CHANNEL
  JsonArray &_channel = jObj.createNestedArray("channel");
  channelAry(_channel, gSystem->temperatures.count());

  //JsonObject& _master = jObj.createNestedObject("pitmaster");

  // PITMASTER  (Cloud kann noch kein Array verarbeiten)
  if (cloud)
  {
    JsonArray &_master = jObj.createNestedArray("pitmaster");
    pitAry(_master, gSystem->pitmasters.count());
  }
  else
  {
    JsonObject &_master = jObj.createNestedObject("pitmaster");
    pitTyp(_master);
    JsonArray &_pit = _master.createNestedArray("pm");
    pitAry(_pit, gSystem->pitmasters.count());
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SETTINGS JSON Object
void API::settingsObj(JsonObject &jObj)
{
  // SYSTEM
  JsonObject &_system = jObj.createNestedObject("system");
  systemObj(_system, true);

  JsonArray &_hw = jObj.createNestedArray("hardware");
  _hw.add(String("V") + String(gSystem->getHardwareVersion()));

  JsonObject &api = jObj.createNestedObject("api");
  api["version"] = GUIAPIVERSION;

  // SENSORS
  JsonArray &_sensorsArray = jObj.createNestedArray("sensors");
  for (uint8_t i = 0; i < NUM_OF_TYPES; i++)
  {
    JsonObject &_sensorObject = _sensorsArray.createNestedObject();
    _sensorObject["type"] = (uint8_t)sensorTypeInfo[i].type;
    _sensorObject["name"] = sensorTypeInfo[i].name;
    _sensorObject["fixed"] = sensorTypeInfo[i].fixed;
  }

  // FEATURES
  JsonObject &_features = jObj.createNestedObject("features");
  _features["bluetooth"] = (gSystem->bluetooth) ? (gSystem->bluetooth->isBuiltIn()) : false;
  _features["pitmaster"] = (boolean)(gSystem->pitmasters.count() > 0u);

  // PID-PROFILS
  JsonArray &_pid = jObj.createNestedArray("pid");
  pidAry(_pid, gSystem->pitmasters.count());

  // AKTORS
  JsonArray &_aktor = jObj.createNestedArray("aktor");
  _aktor.add("SSR");
  _aktor.add("FAN");
  _aktor.add("SERVO");
  if (gSystem->getSupportDamper())
  {
    _aktor.add("DAMPER");
  }

  // DISPLAY
  JsonObject &_display = jObj.createNestedObject("display");
  displayObj(_display);

  // IOT
  JsonObject &_iot = jObj.createNestedObject("iot");
  iotObj(_iot);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CLOUD JSON Object - Level 1
void API::cloudObj(JsonObject &jObj)
{
  CloudConfig cloudConfig = gSystem->cloud.getConfig();

  jObj["task"] = "save";
  jObj["api_token"] = cloudConfig.cloudToken;

  JsonArray &data = jObj.createNestedArray("data");
  // aktuelle Werte
  JsonObject &_obj = data.createNestedObject();
  dataObj(_obj, true);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CUSTOM JSON Object - Level 1
void API::customObj(JsonObject &jObj)
{
  CloudConfig cloudConfig = gSystem->cloud.getConfig();

  jObj["version"] = 1;
  jObj["interval"] = cloudConfig.customInterval;

  // CHANNEL
  JsonArray &_channel = jObj.createNestedArray("channel");
  channelAry(_channel, gSystem->temperatures.count());

  for (JsonArray::iterator it = _channel.begin(); it != _channel.end(); ++it)
  {
    JsonObject &_currentChannel = it->asObject();

    _currentChannel["unit"] = String((char)gSystem->temperatures.getUnit());
    
    if(INACTIVEVALUE == _currentChannel["temp"])
    {
      _currentChannel["temp"] = (char*)0;
    }
  }
}


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Hauptprogramm API - JSON Generator
String API::apiData(int typ)
{

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  if ((APIDATA == typ) || (APICUSTOM == typ))
  { //  || typ == APISETTINGS
    // interne Kommunikation mit dem Webinterface
    // oder custom cloud
  }
  else
  {
    JsonObject &device = root.createNestedObject("device");
    deviceObj(device);
  }

  switch (typ)
  {

  case NOAPI:
  {
    return "";
  }

  case APIUPDATE:
  {
    JsonObject &update = root.createNestedObject("update");
    updateObj(update);

    JsonObject &url = root.createNestedObject("url");
    urlObj(url);
    break;
  }

  case APICLOUD:
  {
    JsonObject &cloud = root.createNestedObject("cloud");
    cloudObj(cloud);
    break;
  }

  case APICUSTOM:
  {
    customObj(root);
    break;
  }

  case APIDATA:
  {
    dataObj(root, false);
    break;
  }

  case APISETTINGS:
  {
    settingsObj(root);
    break;
  }

  case APINOTIFICATION:
  {
    JsonObject &note = root.createNestedObject("notification_v2");
    notificationObj(note);
    break;
  }

  case APICRASHREPORT:
  {
    JsonObject &crash = root.createNestedObject("crash_report");
    crashObj(crash);
    break;
  }
  }

  String jsonStr;
  root.printTo(jsonStr);

  return jsonStr;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Nachkommastellen limitieren
float API::limit_float(float f, int i)
{
  if (i >= 0)
  {
    if (gSystem->temperatures[i]->getValue() != INACTIVEVALUE)
    {
      f = f + 0.05;      // damit er "richtig" rundet, bei 2 nachkommastellen 0.005 usw.
      f = (int)(f * 10); // hier wird der float *10 gerechnet und auf int gecastet, so fallen alle weiteren Nachkommastellen weg
      f = f / 10;
    }
    else
      f = 999;
  }
  else
  {
    f = f + 0.005;
    f = (int)(f * 100);
    f = f / 100;
  }
  return f;
}