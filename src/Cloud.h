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
#include <TimeLib.h>
#include <asyncHTTPrequest.h>

typedef struct
{
  bool enabled;
  String token;
  uint16_t interval;
} CloudConfig;

typedef struct
{
  uint8_t urlIndex;
  const char* requestData;
} CloudRequest;

enum
{
  NOAPI,
  APIUPDATE,
  APICLOUD,
  APIDATA,
  APISETTINGS,
  APINOTIFICATION,
  APICRASHREPORT,
};

// URL
struct ServerData
{
  String host; // nur die Adresse ohne Anhang
  String page; // alles was nach de, com etc. kommt
  String typ;
};

enum
{
  APILINK,
  NOTELINK,
  CLOUDLINK
};

class Cloud
{
public:
  Cloud();
  void update();
  void loadConfig();
  void saveConfig();
  void saveUrl();
  CloudConfig getConfig();
  void setConfig(CloudConfig newConfig);
  String newToken();
  uint8_t state;
  static void checkAPI();
  static void sendAPI(int apiIndex, int urlIndex);

  static uint8_t serverurlCount;
  static ServerData serverurl[]; // 0:api, 1: note, 2:cloud
  static bool clientlog;

private:
  String createToken();
  static void onReadyStateChange(void *optParm, asyncHTTPrequest *request, int readyState);
  static void readUTCfromHeader(String payload);
  static tmElements_t *string_to_tm(tmElements_t *tme, char *str);
  void handleQueue();
  CloudConfig config;
  static asyncHTTPrequest apiClient;
  static QueueHandle_t apiQueue;
  uint16_t intervalCounter;
};
