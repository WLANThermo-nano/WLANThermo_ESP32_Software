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
#include "Version.h"
#include "WebHandler.h"
#include "DbgPrint.h"

API::API()
{
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
    jObj["item"] =gSystem->item.read(ItemNvsKeys::kItem);

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
      data["fixed"] = temperature->getFixedSensor();
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
      ma["channel"] = pm->getAssignedTemperature()->getGlobalIndex() + 1;
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
    PitmasterProfile *profile = gSystem->profile[i];

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
  jObj["PMQhost"] = mqttConfig.host;
  jObj["PMQport"] = mqttConfig.port;
  jObj["PMQuser"] = mqttConfig.user;
  jObj["PMQpass"] = mqttConfig.password;
  jObj["PMQqos"] = mqttConfig.QoS;
  jObj["PMQon"] = mqttConfig.enabled;
  jObj["PMQint"] = mqttConfig.interval;
  jObj["CLon"] = cloudConfig.enabled;
  jObj["CLtoken"] = cloudConfig.token;
  jObj["CLint"] = cloudConfig.interval;
  jObj["CLurl"] = "dev-cloud.wlanthermo.de/index.html";
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FCM JSON Array
void API::fcmAry(JsonArray &jAry, int cc)
{

  int i = 0;
  if (cc < 3)
    i = cc - 1; // nur ein bestimmtes Profil

  for (i; i < cc; i++)
  {
    JsonObject &_fcm = jAry.createNestedObject();
    _fcm["id"] = i;
    _fcm["on"] = (byte) true;
    //_fcm["token"] = "cerAGIyShJk:APA91bGX6XYvWm7W-KQN1FUw--IDiceKfKnpa0AZ3B2gNhldbkNkz7c1-Js0ma5QA8v2nBcZsf7ndPEWBGfRogHU6RzOI08IAhOyL5cXpUeAKDOTaO5O6XMHq89IHh8UaycRi4evFMbM";
    _fcm["pseudo"] = "AdminSamsung";
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Update JSON Object
void API::extObj(JsonObject &jObj)
{

  jObj["on"] = gSystem->notification.pushService.on;
  jObj["token"] = gSystem->notification.pushService.token;
  jObj["id"] = gSystem->notification.pushService.id;
  jObj["repeat"] = gSystem->notification.pushService.repeat;
  jObj["service"] = gSystem->notification.pushService.service;

  JsonArray &_noteservice = jObj.createNestedArray("services");
  _noteservice.add("telegram"); // 0
  _noteservice.add("pushover"); // 1
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Update JSON Object
void API::updateObj(JsonObject &jObj)
{
  jObj["prerelease"] = gSystem->otaUpdate.getPrerelease();

  // nach einer bestimmten Version fragen
  if (gSystem->otaUpdate.getRequestedVersion() != "false")
    jObj["version"] = gSystem->otaUpdate.getRequestedVersion();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Alexa JSON Object
void API::alexaObj(JsonObject &jObj)
{

  jObj["task"] = "save"; // save or delete
  jObj["token"] = "xxx";
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

  // PID-PROFILS
  JsonArray &_pid = jObj.createNestedArray("pid");
  pidAry(_pid, gSystem->pitmasters.count());

  // AKTORS
  JsonArray &_aktor = jObj.createNestedArray("aktor");
  _aktor.add("SSR");
  _aktor.add("FAN");
  _aktor.add("SERVO");
  /*if (sys.damper)
    _aktor.add("DAMPER");*/
  //TODO

  // IOT
  JsonObject &_iot = jObj.createNestedObject("iot");
  iotObj(_iot);

  // NOTES
  JsonObject &_note = jObj.createNestedObject("notes");
  JsonArray &_firebase = _note.createNestedArray("fcm");
  //fcmAry(_note);

  JsonObject &_ext = _note.createNestedObject("ext");
  extObj(_ext);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CLOUD JSON Object - Level 1
void API::cloudObj(JsonObject &jObj)
{
  CloudConfig cloudConfig = gSystem->cloud.getConfig();

  jObj["task"] = "save";
  jObj["api_token"] = cloudConfig.token;

  JsonArray &data = jObj.createNestedArray("data");
  // aktuelle Werte
  JsonObject &_obj = data.createNestedObject();
  dataObj(_obj, true);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Notification JSON Object
void API::noteObj(JsonObject &jObj)
{
  CloudConfig cloudConfig = gSystem->cloud.getConfig();

  jObj["lang"] = gSystem->getLanguage();

  if (cloudConfig.enabled)
    jObj["api_token"] = cloudConfig.token;

  // an welche Dienste soll geschickt werden?
  JsonArray &services = jObj.createNestedArray("services");

  JsonObject &_obj1 = services.createNestedObject();
  switch (gSystem->notification.pushService.service)
  {
  case 0:
    _obj1["service"] = F("telegram");
    break;
  case 1:
    _obj1["service"] = F("pushover");
    break;
  case 2:
    _obj1["service"] = F("prowl");
    break;
  }
  _obj1["key1"] = gSystem->notification.pushService.token;
  _obj1["key2"] = gSystem->notification.pushService.id;

  // gSystem->notification.pushService.repeat;

  // Nachricht
  jObj["task"] = "alert";

  if (gSystem->notification.notificationData.type == 1)
  {
    jObj["message"] = "test";
    if (gSystem->notification.pushService.on == 2)
      gSystem->notification.pushService.on = 3; // alte Werte wieder herstellen  (Testnachricht)
  }
  else if (gSystem->notification.notificationData.type == 2)
  {
    jObj["message"] = "battery";
  }
  else
  {

    jObj["unit"] = String((char)gSystem->temperatures.getUnit());

    bool limit = gSystem->notification.notificationData.limit & (1 << gSystem->notification.notificationData.ch);
    jObj["message"] = (limit) ? F("up") : F("down");
    jObj["channel"] = gSystem->notification.notificationData.ch + 1;
    gSystem->notification.notificationData.index &= ~(1 << gSystem->notification.notificationData.ch); // Kanal entfernen, sonst erneuter Aufruf

    TemperatureBase *temperature = gSystem->temperatures[gSystem->notification.notificationData.ch];
    if (temperature != NULL)
    {
      temperature->updateNotificationCounter();
      JsonArray &_temp = jObj.createNestedArray("temp");
      _temp.add(temperature->getValue());
      if (limit)
        _temp.add(temperature->getMaxValue());
      else
        _temp.add(temperature->getMinValue());
    }
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Hauptprogramm API - JSON Generator
String API::apiData(int typ)
{

  DynamicJsonBuffer jsonBuffer;
  JsonObject &root = jsonBuffer.createObject();

  if (typ == APIDATA)
  { //  || typ == APISETTINGS
    // interne Kommunikation mit dem Webinterface
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

  case APINOTE:
  {
    JsonObject &note = root.createNestedObject("notification");
    noteObj(note);
    gSystem->notification.notificationData.type = 0; // Zuruecksetzen
    break;
  }

  case APIALEXA:
  {
    JsonObject &alexa = root.createNestedObject("alexa");
    alexaObj(alexa);
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