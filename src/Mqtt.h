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
#pragma once

#include <Arduino.h>
#include <AsyncMqttClient.h>
#include "Settings.h"

#define MQTT_STRING_SIZE 30u

typedef struct
{
  char host[MQTT_STRING_SIZE];
  uint16_t port;
  char user[MQTT_STRING_SIZE];
  char password[MQTT_STRING_SIZE];
  byte QoS;
  bool enabled;
  int interval;
} MqttConfig;

class Mqtt
{
public:
  Mqtt();
  void update();
  void saveConfig();
  void loadConfig();
  MqttConfig getConfig();
  void setConfig(MqttConfig newConfig);

private:
  static bool sendSettings();
  static bool sendData();
  static void onSettingsWrite(SettingsNvsKeys key);

  static void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
  static void onMqttConnect(bool sessionPresent);
  static void onMqttSubscribe(uint16_t packetId, uint8_t qos);
  static void onMqttUnsubscribe(uint16_t packetId);
  static void onMqttMessage(char *topic, char *datas, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
  static void onMqttPublish(uint16_t packetId);

  static AsyncMqttClient pmqttClient;
  static MqttConfig config;
  static bool sendSettingsflag;
  static uint16_t intervalCounter;
  bool initDone;
};
