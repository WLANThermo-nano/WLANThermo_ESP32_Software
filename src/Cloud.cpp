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
#include "Cloud.h"
#include "system/SystemBase.h"
#include "Settings.h"
#include "API.h"
#include "WebHandler.h"
#include "DbgPrint.h"
#include <SPIFFS.h>

// API
#define APISERVER "api.wlanthermo.de"
#define CHECKAPI "/"
#define URL_FILE "/url.json"
#define DEFAULT_INTERVAL 30u

#define READY_STATE_UNSENT 0
#define READY_STATE_OPENED 1
#define READY_STATE_HEADERS_RECEIVED 2
#define READY_STATE_LOADING 3
#define READY_STATE_DONE 4

#define HTTP_STATUS_OK 200

#define API_QUEUE_SIZE 10u

enum
{
  GETMETH,
  POSTMETH
};

uint8_t Cloud::serverurlCount = 3u;
ServerData Cloud::serverurl[3] = {
    {APISERVER, CHECKAPI, "api"},
    {APISERVER, CHECKAPI, "note"},
    {APISERVER, CHECKAPI, "cloud"}};

asyncHTTPrequest Cloud::apiClient = asyncHTTPrequest();
QueueHandle_t Cloud::apiQueue = xQueueCreate(API_QUEUE_SIZE, sizeof(CloudData));
bool Cloud::clientlog = false;

enum
{
  CONNECTFAIL,
  SENDTO,
  DISCONNECT,
  CLIENTCONNECT
};

String printDigits(int digits)
{
  String com;
  if (digits < 10)
    com = "0";
  com += String(digits);
  return com;
}

String digitalClockDisplay(time_t t)
{
  String zeit;
  zeit += printDigits(hour(t)) + ":";
  zeit += printDigits(minute(t)) + ":";
  zeit += printDigits(second(t)) + " ";
  zeit += String(day(t)) + ".";
  zeit += String(month(t)) + ".";
  zeit += String(year(t));
  return zeit;
}

Cloud::Cloud()
{
  config.enabled = false;
  config.token = createToken();
  config.interval = DEFAULT_INTERVAL;
  intervalCounter = 0u;
  state = 0;
}

void Cloud::update()
{
  if (config.enabled)
  {
    // First get time from server before sending data
    if (now() < 31536000)
    {
      Cloud::sendAPI(NOAPI, APILINK, NOPARA);
    }
    else if (0u == intervalCounter)
    {
      intervalCounter = config.interval;
      Cloud::sendAPI(APICLOUD, CLOUDLINK, NOPARA);
    }
  }
  else
  {
    gSystem->cloud.state = 0;
  }

  handleQueue();

  if (intervalCounter)
    intervalCounter--;
}

String Cloud::newToken()
{
  config.token = createToken();
  return config.token;
}

String Cloud::createToken()
{
  String stamp = String(now(), HEX);
  int x = 10 - stamp.length(); //pow(16,(10 - timestamp.length()));
  long y = 1;                  // long geht bis 16^7
  if (x > 7)
  {
    stamp += String(random(268435456), HEX);
    x -= 7;
  }
  for (int i = 0; i < x; i++)
    y *= 16;
  stamp += String(random(y), HEX);
  return (String)(gSystem->getSerialNumber() + stamp);
}

void Cloud::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();
  json["enabled"] = config.enabled;
  json["token"] = config.token;
  json["interval"] = config.interval;
  Settings::write(kCloud, json);

  // trigger send after config update
  intervalCounter = 0u;
}

void Cloud::saveUrl()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();

  for (uint8_t i = 0; i < Cloud::serverurlCount; i++)
  {

    JsonObject &_obj = json.createNestedObject(serverurl[i].typ);
    _obj["host"] = serverurl[i].host;
    _obj["page"] = serverurl[i].page;
  }

  File file = SPIFFS.open(URL_FILE, "w");

  if (file)
  {
    json.printTo(file);
    file.close();
  }
}

void Cloud::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kCloud, &jsonBuffer);

  if (json.success())
  {
    if (json.containsKey("enabled"))
      config.enabled = json["enabled"];
    if (json.containsKey("token"))
      config.token = json["token"].asString();
    if (json.containsKey("interval"))
      config.interval = json["interval"];
  }

  File file = SPIFFS.open(URL_FILE, "r");

  if (file)
  {
    String jsonString = file.readString();
    Serial.printf("url.json: %s\n", jsonString.c_str());
    JsonObject &json = jsonBuffer.parseObject(file.readString().c_str());

    if (json.success())
    {
      for (int i = 0; i < Cloud::serverurlCount; i++)
      {
        JsonObject &_link = json[Cloud::serverurl[i].typ];

        if (_link.containsKey("host"))
          Cloud::serverurl[i].host = _link["host"].asString();
        else
          break;

        if (_link.containsKey("page"))
          Cloud::serverurl[i].page = _link["page"].asString();
      }
    }

    file.close();
  }
}

CloudConfig Cloud::getConfig()
{
  return config;
}

void Cloud::setConfig(CloudConfig newConfig)
{
  // copy new config
  config = newConfig;

  // trigger send after config update
  intervalCounter = 0u;

  // save to NvM
  saveConfig();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Read time stamp from HTTP Header
void Cloud::readUTCfromHeader(String payload)
{
  // Jahr 1971
  if (now() < 31536000)
  {
    tmElements_t tmx;
    string_to_tm(&tmx, (char *)payload.c_str());
    setTime(makeTime(tmx));

    IPRINTP("UTC: ");
    DPRINTLN(digitalClockDisplay(now()));
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Date String to Date Element
// Quelle: https://github.com/oh1ko/ESP82666_OLED_clock/blob/master/ESP8266_OLED_clock.ino
tmElements_t *Cloud::string_to_tm(tmElements_t *tme, char *str)
{
  // Sat, 28 Mar 2015 13:53:38 GMT

  const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  char *r, *i, *t;
  r = strtok_r(str, " ", &i);

  r = strtok_r(NULL, " ", &i);
  tme->Day = atoi(r);

  r = strtok_r(NULL, " ", &i);
  for (int i = 0; i < 12; i++)
  {
    if (!strcmp(months[i], r))
    {
      tme->Month = i + 1;
      break;
    }
  }

  r = strtok_r(NULL, " ", &i);
  tme->Year = atoi(r) - 1970;

  r = strtok_r(NULL, " ", &i);
  t = strtok_r(r, ":", &i);
  tme->Hour = atoi(t);

  t = strtok_r(NULL, ":", &i);
  tme->Minute = atoi(t);

  t = strtok_r(NULL, ":", &i);
  tme->Second = atoi(t);

  return tme;
}

void Cloud::onReadyStateChange(void *optParm, asyncHTTPrequest *request, int readyState)
{
  boolean *requestDone = (boolean *)optParm;

  if (READY_STATE_DONE == readyState)
  {
    if (request->respHeaderExists("Date"))
      readUTCfromHeader(request->respHeaderValue("Date"));

    if(request->responseHTTPcode() == HTTP_STATUS_OK)
      nanoWebHandler.setServerAPI(NULL, (uint8_t *)request->responseText().c_str());
    
    *requestDone = true;
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Send to API
void Cloud::sendAPI(int apiIndex, int urlIndex, int parIndex)
{
  CloudData data = {apiIndex, urlIndex, parIndex};
  xQueueSend(apiQueue, &data, 0u);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Handle API queue
void Cloud::handleQueue()
{
  static boolean requestDone = true;

  CloudData cloudData;

  if (requestDone)
  {
    if(xQueueReceive(apiQueue, &cloudData, 0u) == pdFALSE)
      return;

    requestDone = false;

    if (clientlog)
      apiClient.setDebug(true);

    apiClient.onReadyStateChange(Cloud::onReadyStateChange, &requestDone);
    apiClient.open("POST", String("http://" + serverurl[cloudData.urlIndex].host + "/").c_str());
    apiClient.setReqHeader("Connection", "close");
    apiClient.setReqHeader("User-Agent", "WLANThermo ESP32");
    apiClient.setReqHeader("SN", gSystem->getSerialNumber().c_str());
    apiClient.send(API::apiData(cloudData.apiIndex));
  }
}