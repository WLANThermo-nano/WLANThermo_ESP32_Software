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
  boolean enabled;
  String token;
  uint16_t interval;
} CloudConfig;

enum class Url
{
  Api,
  Note,
  Cloud
};

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

typedef struct
{
  uint8_t apiIndex;
  Url urlIndex;
} CloudData;

class Cloud
{
public:
  Cloud();
  void update();
  void loadConfig();
  void saveConfig();
  CloudConfig getConfig();
  void setConfig(CloudConfig newConfig);
  String newToken();
  uint8_t state;
  static void checkAPI();
  static void sendAPI(uint8_t apiIndex, Url urlIndex);

  static bool clientlog;

private:
  String createToken();
  String getUrl(Url cloudUrl);
  void loadUrlConfig();
  static void onReadyStateChange(void *optParm, asyncHTTPrequest *request, int readyState);
  static void readUTCfromHeader(String payload);
  static tmElements_t *string_to_tm(tmElements_t *tme, char *str);
  void handleQueue();
  CloudConfig config;
  static asyncHTTPrequest apiClient;
  static QueueHandle_t apiQueue;
  uint16_t intervalCounter;
  boolean devUrlEnabled;
  String noteUrl;
  String cloudUrl;
  String userNoteUrl;
  String userCloudUrl;
};
