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
#include "Mqtt.h"
#include "system/SystemBase.h"
#include "DbgPrint.h"
#include "WebHandler.h"
#include "API.h"

AsyncMqttClient Mqtt::pmqttClient;
MqttConfig Mqtt::config = {"192.168.2.1", 1883u, "", "", 0, false, 30};
bool Mqtt::sendSettingsflag = false;
uint16_t Mqtt::intervalCounter = 0u;

Mqtt::Mqtt()
{
  initDone = false;
}

void Mqtt::update()
{
  if (0u == intervalCounter)
  {
    intervalCounter = config.interval;

    if (gSystem->mqtt.config.enabled && pmqttClient.connected())
    {
      if (sendSettingsflag)
      {
        sendSettings();
        sendSettingsflag = false;
      }

      sendData();
    }
    else if (gSystem->mqtt.config.enabled)
    {
      if (false == initDone)
      {
        initDone = true;

        pmqttClient.onConnect(onMqttConnect);
        pmqttClient.onDisconnect(onMqttDisconnect);
        pmqttClient.onSubscribe(onMqttSubscribe);
        pmqttClient.onUnsubscribe(onMqttUnsubscribe);
        pmqttClient.onMessage(onMqttMessage);
        pmqttClient.onPublish(onMqttPublish);

        Settings::onWrite(onSettingsWrite);

        pmqttClient.setServer(gSystem->mqtt.config.host, gSystem->mqtt.config.port);

        if (strlen(gSystem->mqtt.config.user) && strlen(gSystem->mqtt.config.password))
          pmqttClient.setCredentials(gSystem->mqtt.config.user, gSystem->mqtt.config.password);
      }

      pmqttClient.connect();
    }
  }

  if (intervalCounter)
    intervalCounter--;
}

void Mqtt::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();
  json["host"] = config.host;
  json["port"] = config.port;
  json["user"] = config.user;
  json["password"] = config.password;
  json["QoS"] = config.QoS;
  json["enabled"] = config.enabled;
  json["interval"] = config.interval;
  Settings::write(kMqtt, json);
}

void Mqtt::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kMqtt, &jsonBuffer);

  if (json.success())
  {

    if (json.containsKey("host"))
      strcpy(config.host, json["host"].asString());
    if (json.containsKey("port"))
      config.port = json["port"];
    if (json.containsKey("user"))
      strcpy(config.user, json["user"].asString());
    if (json.containsKey("password"))
      strcpy(config.password, json["password"].asString());
    if (json.containsKey("QoS"))
      config.QoS = json["QoS"];
    if (json.containsKey("enabled"))
      config.enabled = json["enabled"];
    if (json.containsKey("interval"))
      config.interval = json["interval"];
  }
}

MqttConfig Mqtt::getConfig()
{
  return config;
}

void Mqtt::setConfig(MqttConfig newConfig)
{
  // disconnect before updating config
  if ((true == gSystem->mqtt.config.enabled) && pmqttClient.connected())
    pmqttClient.disconnect();

  // copy new config
  config = newConfig;

  // trigger send after config update
  intervalCounter = 0u;

  // update MQTT server settings
  pmqttClient.setServer(gSystem->mqtt.config.host, gSystem->mqtt.config.port);

  // update credentials
  if (strlen(gSystem->mqtt.config.user) && strlen(gSystem->mqtt.config.password))
    pmqttClient.setCredentials(gSystem->mqtt.config.user, gSystem->mqtt.config.password);

  // save to NvM
  saveConfig();
}

void Mqtt::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  IPRINTPLN("d:MQTT");
  sendSettingsflag = false;
}

void Mqtt::onMqttConnect(bool sessionPresent)
{
  IPRINTPLN("c:MQTT");
  MQPRINTP("[MQTT]\tSession present: ");
  MQPRINTLN(sessionPresent);
  String adress = F("WLanThermo/");
  adress += gSystem->wlan.getHostName();
  adress += F("/#");
  uint16_t packetIdSub = pmqttClient.subscribe(adress.c_str(), 2);
  MQPRINTP("[MQTT]\tSubscribing, packetId: ");
  MQPRINTLN(packetIdSub);
  sendSettingsflag = true;
  intervalCounter = 0u;
}

void Mqtt::onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
  MQPRINTPLN("[MQTT]\tSubscribe acknowledged.");
  MQPRINTP("packetId: ");
  MQPRINTLN(packetId);
  MQPRINTP("qos: ");
  MQPRINTLN(qos);
}

void Mqtt::onMqttUnsubscribe(uint16_t packetId)
{
  MQPRINTPLN("[MQTT]\tUnsubscribe acknowledged.");
  MQPRINTP("packetId: ");
  MQPRINTLN(packetId);
}

void Mqtt::onMqttMessage(char *topic, char *datas, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  String topic_prefix = F("WLanThermo/");
  topic_prefix += gSystem->wlan.getHostName();
  int topic_prefix_length = topic_prefix.length();
  String topic_short = String(topic);
  topic_short.remove(0, topic_prefix_length);

  if (topic_short.startsWith("/set/channels"))
  {
    nanoWebHandler.setChannels(NULL, (uint8_t *)datas);
  }
  if (topic_short.startsWith("/set/system"))
  {
    nanoWebHandler.setSystem(NULL, (uint8_t *)datas);
  }
  if (topic_short.startsWith("/set/pitmaster"))
  {
    nanoWebHandler.setPitmaster(NULL, (uint8_t *)datas);
  }
  if (topic_short.startsWith("/set/pid"))
  {
    nanoWebHandler.setPID(NULL, (uint8_t *)datas);
  }
  if (topic_short.startsWith("/set/iot"))
  {
    nanoWebHandler.setIoT(NULL, (uint8_t *)datas);
  }
  if (topic_short.startsWith("/get/settings"))
  {
    sendSettings();
  }
  if (topic_short.startsWith("/get/data"))
  {
    sendData();
  }
  // placeholder for future extensions
  // if (topic_short.startsWith("/cmd/action")) {
  // dummy_action_handler();
  //}
}

void Mqtt::onMqttPublish(uint16_t packetId)
{
  MQPRINTPLN("[MQTT]\tPublish acknowledged.");
  MQPRINTP("  packetId: ");
  MQPRINTLN(packetId);
}

String prefixgen(uint8_t stil = 0)
{
  String prefix = F("WLanThermo/");
  prefix += gSystem->wlan.getHostName();

  switch (stil)
  {
  case 1:
    return prefix + F("/status/data");
  case 2:
    return prefix + F("/status/settings");
  default:
    return prefix + F("/#");
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// send datas
bool Mqtt::sendData()
{

  if (pmqttClient.connected())
  {
    String payload_data = API::apiData(APIDATA);
    pmqttClient.publish(prefixgen(1).c_str(), gSystem->mqtt.config.QoS, false, payload_data.c_str());
    MQPRINTPLN("[MQTT] Send: /data ");
    return true;
  }
  else
  {
    return false;
  }
}
// send settings
bool Mqtt::sendSettings()
{

  if (pmqttClient.connected())
  {
    String payload_settings = API::apiData(APISETTINGS);
    pmqttClient.publish(prefixgen(2).c_str(), gSystem->mqtt.config.QoS, false, payload_settings.c_str());
    MQPRINTPLN("[MQTT] Send: /settings ");
    return true;
  }
  else
  {
    return false;
  }
}

void Mqtt::onSettingsWrite(SettingsNvsKeys key)
{
  // enable sending of settings
  sendSettingsflag = true;
}