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
#include <AsyncTCP.h>

typedef struct
{
  bool enabled;
  String token;
  uint16_t interval;
} CloudConfig;

typedef struct
{
  int apiIndex;
  int urlIndex;
  int parIndex;
} CloudData;

enum
{
  NOAPI,
  APIUPDATE,
  APICLOUD,
  APIDATA,
  APISETTINGS,
  APINOTE,
  APIALEXA
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

enum
{
  NOPARA,
  TESTPARA,
  SENDTS,
  THINGHTTP
}; // Config GET/POST Request

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
  static bool sendAPI(int apiIndex, int urlIndex, int parIndex);

  static uint8_t serverurlCount;
  static ServerData serverurl[]; // 0:api, 1: note, 2:cloud
  static bool clientlog;

private:
  String createToken();
  static void readUTCfromHeader(String payload);
  static tmElements_t *string_to_tm(tmElements_t *tme, char *str);
  static void readContentLengthfromHeader(String payload, int len);
  static void checkContentTypfromHeader(String payload, int len);
  static void readLocation(String payload, int len);
  static void printClient(const char *link, int arg);
  static String createCommand(bool meth, int para, const char *link, const char *host, int content);

  CloudConfig config;
  static int log_length;
  static int log_typ;
  static int apicontent;
  static AsyncClient *apiClient;
  uint16_t intervalCounter;
};
