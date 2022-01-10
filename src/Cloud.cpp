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
#include "RecoveryMode.h"
#include "API.h"
#include "WebHandler.h"
#include "DbgPrint.h"
#include "ArduinoLog.h"
#include <SPIFFS.h>
#include "ArduinoLog.h"

// API
#define APISERVER "api.wlanthermo.de"
#define CHECKAPI "/"
#define URL_FILE "/url.json"
#define DEFAULT_INTERVAL 30u
#define TOKEN_BYTE_LENGTH 11u
#define TOKEN_STRING_LENGTH ((2u * TOKEN_BYTE_LENGTH) + 1u)

#define READY_STATE_UNSENT 0
#define READY_STATE_OPENED 1
#define READY_STATE_HEADERS_RECEIVED 2
#define READY_STATE_LOADING 3
#define READY_STATE_DONE 4

#define HTTP_STATUS_OK 200

#define API_QUEUE_SIZE 5u

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
QueueHandle_t Cloud::apiQueue = xQueueCreate(API_QUEUE_SIZE, sizeof(CloudRequest));
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
  config.cloudEnabled = false;
  config.cloudToken = createToken();
  config.cloudInterval = DEFAULT_INTERVAL;
  config.customEnabled = false;
  config.customInterval = DEFAULT_INTERVAL;
  config.customUrl = "";
  cloudCounter = 0u;
  customCounter = 0u;
  state = 0u;
}

void Cloud::update()
{
  if (config.cloudEnabled)
  {
    // First get time from server before sending data
    // Also send crash report if enabled
    if (now() < 31536000)
    {
      Cloud::sendAPI(NOAPI, APILINK);
      if(gSystem->getCrashReport() && (RecoveryMode::getResetCounter() > 0u))
      {
        Cloud::sendAPI(APICRASHREPORT, APILINK);
        Log.error("Crash report sent!" CR);
      }
    }
    else if (0u == cloudCounter)
    {
      cloudCounter = config.cloudInterval;
      Cloud::sendAPI(APICLOUD, CLOUDLINK);
    }
  }
  else
  {
    gSystem->cloud.state = 0;
  }

  if (config.customEnabled && config.customUrl.length() > 0u)
  {
    if (0u == customCounter)
    {
      customCounter = config.customInterval;
      Cloud::sendAPI(APICUSTOM, CUSTOMLINK);
    }
  }

  handleQueue();

  if (cloudCounter)
    cloudCounter--;

  if (customCounter)
    customCounter--;
}

String Cloud::newToken()
{
  config.cloudToken = createToken();
  return config.cloudToken;
}

String Cloud::createToken()
{
  uint8_t random[TOKEN_BYTE_LENGTH] = {0u};
  char token[TOKEN_STRING_LENGTH] = {0};

  // use hardware RNG
  esp_fill_random(random, TOKEN_BYTE_LENGTH);

  // copy byte array to hex string
  for(uint8_t index = 0u; index < TOKEN_BYTE_LENGTH; index++)
  {
    sprintf(&token[index*2u], "%02x", random[index]);
  }

  return String(token);
}

void Cloud::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();
  json["enabled"] = config.cloudEnabled;
  json["token"] = config.cloudToken;
  json["interval"] = config.cloudInterval;

  json["customEnabled"] = config.customEnabled;
  json["customUrl"] = config.customUrl;
  json["customInterval"] = config.customInterval;
  Settings::write(kCloud, json);

  // trigger send after config update
  cloudCounter = 0u;
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
      config.cloudEnabled = json["enabled"];
    if (json.containsKey("token"))
      config.cloudToken = json["token"].asString();
    if (json.containsKey("interval"))
      config.cloudInterval = json["interval"];

    if (json.containsKey("customEnabled"))
      config.customEnabled = json["customEnabled"];
    if (json.containsKey("customUrl"))
      config.customUrl = json["customUrl"].asString();
    if (json.containsKey("customInterval"))
      config.customInterval = json["customInterval"];
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
  cloudCounter = 0u;

  // save to NvM
  saveConfig();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Read time stamp from HTTP Header
void Cloud::readUTCfromHeader(String payload)
{
    tmElements_t tmx;
    time_t seconds;
    time_t delta;

    string_to_tm(&tmx, (char *)payload.c_str());
    seconds = makeTime(tmx);
    delta = abs(seconds - now());

    if(delta > 1u)
    {
      setTime(seconds);
      Log.notice("Updated time from http header. New UTC time: %s" CR, digitalClockDisplay(now()).c_str());
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
  int responseCode;

  if (READY_STATE_DONE == readyState)
  {
    if (request->respHeaderExists("Date"))
      readUTCfromHeader(request->respHeaderValue("Date"));

    responseCode = request->responseHTTPcode();

    if (HTTP_STATUS_OK == responseCode)
    {
      nanoWebHandler.setServerAPI(NULL, (uint8_t *)request->responseText().c_str());
    }
    else
    {
      Log.warning("API response HTTP code: %d" CR, responseCode);
    }
    
    *requestDone = true;
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Send to API
void Cloud::sendAPI(int apiIndex, int urlIndex)
{
  String requestDataString = API::apiData(apiIndex);
  char *requestDataPointer = new char[requestDataString.length() + 1u];
  
  if(requestDataPointer != NULL)
  {
    strcpy(requestDataPointer, requestDataString.c_str());
    CloudRequest cloudRequest = {urlIndex, requestDataPointer};
    if(xQueueSend(apiQueue, &cloudRequest, 0u) != pdTRUE)
    {
      delete cloudRequest.requestData;
      Log.warning("Cloud request queue full!" CR);
    }
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Handle API queue
void Cloud::handleQueue()
{
  static boolean requestDone = true;

  CloudRequest cloudRequest;

  if (requestDone)
  {
    if (xQueueReceive(apiQueue, &cloudRequest, 0u) == pdFALSE)
      return;

    requestDone = false;

    if (clientlog)
      apiClient.setDebug(true);

    String url = (cloudRequest.urlIndex != CUSTOMLINK) ? String("http://" + serverurl[cloudRequest.urlIndex].host + "/") : config.customUrl;

    apiClient.onReadyStateChange(Cloud::onReadyStateChange, &requestDone);
    apiClient.open("POST", url.c_str());
    apiClient.setReqHeader("Connection", "close");
    apiClient.setReqHeader("User-Agent", "WLANThermo ESP32");
    apiClient.setReqHeader("Content-Type", "application/json");

    if(cloudRequest.urlIndex != CUSTOMLINK)
      apiClient.setReqHeader("SN", gSystem->getSerialNumber().c_str());
    
    apiClient.send(cloudRequest.requestData);
    delete cloudRequest.requestData;
  }
}