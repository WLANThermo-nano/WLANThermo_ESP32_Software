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

int Cloud::log_length = 0;
int Cloud::log_typ = 0;
int Cloud::apicontent = 0;
AsyncClient *Cloud::apiClient = NULL;
byte Cloud::server_state = NULL;
bool Cloud::clientlog = false;
int Cloud::apiindex = 0;
int Cloud::urlindex = 0;
int Cloud::parindex = 0;

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
      if (Cloud::sendAPI(0))
      {
        Cloud::apiindex = NOAPI;
        Cloud::urlindex = APILINK;
        Cloud::parindex = NOPARA;
        Cloud::sendAPI(2);
      }
    }
    else if (0u == intervalCounter)
    {
      intervalCounter = config.interval;

      if (Cloud::sendAPI(0))
      {
        Cloud::apiindex = APICLOUD;
        Cloud::urlindex = CLOUDLINK;
        Cloud::parindex = NOPARA;
        Cloud::sendAPI(2);
      }
    }
  }
  else
  {
    gSystem->cloud.state = 0;
  }

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
  int index = payload.indexOf("Date: ");

  // Jahr 1971
  if (index > -1 && now() < 31536000)
  {

    char date_string[27];
    for (int i = 0; i < 26; i++)
    {
      char c = payload[index + i + 6];
      date_string[i] = c;
    }

    tmElements_t tmx;
    string_to_tm(&tmx, date_string);
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

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Read content length from HTTP Header
void Cloud::readContentLengthfromHeader(String payload, int len)
{

  log_length = 0;
  int index = payload.indexOf("Content-Length: ");
  if (index > -1)
  {

    payload = payload.substring(index + 16, len);          // "Content-Length:" entfernen
    payload = payload.substring(0, payload.indexOf("\n")); // Ende der Zeile
    log_length = payload.toInt();
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Check content typ from HTTP Header
void Cloud::checkContentTypfromHeader(String payload, int len)
{

  int index = payload.indexOf("Content-Type: ");
  if (index > -1)
  {

    payload = payload.substring(index + 14, len);          // "Content-Length:" entfernen
    payload = payload.substring(0, payload.indexOf("\n")); // Ende der Zeile

    if (payload.indexOf("json") > -1)
      log_typ = 1; // JSON
    else if (payload.indexOf("text") > -1)
      log_typ = 2; // TEXT
    else
      log_typ = 0;
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Read new location from 302 HTTP Header
void Cloud::readLocation(String payload, int len)
{

  int index = payload.indexOf("Location: ");
  if (index > -1)
  {
    payload = payload.substring(index + 10, len);          // "Location" entfernen
    payload = payload.substring(0, payload.indexOf("\n")); // Ende des Links
    Serial.println(payload);

    index = payload.indexOf("?"); // Eventuelle Anhänge entfernen
    if (index > -1)
      payload = payload.substring(0, index);
    len = payload.length();
    index = payload.indexOf("://"); // http entfernen
    if (index > -1)
      payload = payload.substring(index + 3, len);
    index = payload.indexOf("/");

    if (index > -1)
    {
      serverurl[APILINK].host = payload.substring(0, index);
      serverurl[APILINK].page = payload.substring(index, len);
      //setconfig(eSERVER, {}); // für Serverlinks TODO
      gSystem->restart();
    }
  }
}

void Cloud::printClient(const char *link, int arg)
{

  switch (arg)
  {
  case CONNECTFAIL:
    IPRINTP("f:");
    break; // Client Connect Fail
  case SENDTO:
    IPRINTP("s:");
    break; // Client Send to
  case DISCONNECT:
    IPRINTP("d:");
    break; // Disconnect Client
  case CLIENTCONNECT:
    IPRINTP("c:");
    break; // Client Connect
  }
  DPRINTLN(link);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Send to API
bool Cloud::sendAPI(int check)
{

  if (check == 0)
  {
    if (apiClient)
      return false; //client already exists

    apiClient = new AsyncClient();
    if (!apiClient)
      return false; //could not allocate client

    apiClient->onError([](void *arg, AsyncClient *client, int error) {
      IPRINTP("e:Client");
      apiClient = NULL;
      delete client;
    },
                       NULL);
  }
  else if (check == 2)
  { // Senden ueber Server

    server_state = 0;
    apiClient->onConnect([](void *arg, AsyncClient *client) {
      //printClient(serverurl[urlindex].page.c_str(),CLIENTCONNECT);
      apicontent = false;

      apiClient->onError(NULL, NULL);

      client->onDisconnect([](void *arg, AsyncClient *c) {
        //printClient(serverurl[urlindex].page.c_str() ,DISCONNECT);
        apiClient = NULL;
        delete c;
      },
                           NULL);

      client->onData([](void *arg, AsyncClient *c, void *data, size_t len) {
        String payload((char *)data);
        //Serial.println(millis());
        if (clientlog)
          Serial.println(payload);
        //Serial.println(len);

        if (payload.indexOf("HTTP/1.1") > -1)
        { // Time Stamp
          readUTCfromHeader(payload);
        }

        if ((payload.indexOf("200 OK") > -1))
        {                   // 200 Header
          server_state = 1; // Server Communication: yes
          readContentLengthfromHeader(payload, len);
          checkContentTypfromHeader(payload, len);

          if (log_length > 0)
          { // Content available
            apicontent = 1;
            if (log_typ == 1 && payload.indexOf("{") > -1)
            { // JSON: Body belongs to header
              apicontent = payload.indexOf("{") + 1;
              if ((len - (apicontent - 1)) != log_length)
                Serial.println("[WARNING]: Content-Length unequal");
              Serial.println("Body belongs to header");
            }
            else
              return; // Header alone
          }
        }
        else if (payload.indexOf("302 Found") > -1)
        { // 302 Header: new API-Links
          readLocation(payload, len);
        }

        //} else if (payload.indexOf("500 Internal Server Error") > -1) {  // 500 Header: new API-Links
        //  Serial.println("Fehler im Verbindungsaufbau");

        if (apicontent)
        { // Body: 1 part
          if (log_typ == 1)
          { // JSON
            bodyWebHandler.setServerAPI(NULL, (uint8_t *)data + (apicontent - 1));
          }
          apicontent = 0;
          log_length -= len; // Option das nicht alles auf einmal kommt bleibt offen
          //Serial.println(log_length);
        }
        else if (log_length > 0)
        {                    // Body: current part
          log_length -= len; // leeren?
          //Serial.println(log_length);
        }
      },
                     NULL);

      //send the request
      //printClient(serverurl[urlindex].page.c_str() ,SENDTO);
      String message = API::apiData(apiindex);
      String adress = createCommand(POSTMETH, parindex, serverurl[urlindex].page.c_str(), serverurl[urlindex].host.c_str(), message.length());
      adress += message;
      client->write(adress.c_str());
      if (clientlog)
        Serial.println(adress);
      apiindex = 0;
      urlindex = 0;
      parindex = 0;
    },
                         NULL);

    if (!apiClient->connect(serverurl[urlindex].host.c_str(), 80))
    {
      printClient(serverurl[urlindex].page.c_str(), CONNECTFAIL);
      AsyncClient *client = apiClient;
      apiClient = NULL;
      delete client;
    }
  }
  return true; // Nachricht kann gesendet werden
}

void Cloud::check_api()
{

  // bei wifi-connect wird gSystem->otaUpdate.state = -1 gesetzt

  if (gSystem->otaUpdate.state == -1 || gSystem->otaUpdate.state == 2)
  { // -1 = check, 2 = check after restart during update
    if ((gSystem->wlan.isConnected()))
    {
      //Serial.println("Verbindungsversuch API");
      if (sendAPI(0))
      { // blockt sich selber, so dass nur ein Client gleichzeitig offen
        apiindex = APIUPDATE;
        urlindex = APILINK;
        parindex = NOPARA;
        sendAPI(2);
      }

      // kommt nicht bei Systemstart zum Einsatz
    }
    else
    { // kein Internet, also Update stoppen
      if (gSystem->otaUpdate.state == -1)
        gSystem->otaUpdate.get = "false"; // nicht während Update, da wird die Version gebraucht
    }

    if (gSystem->otaUpdate.state == -1)
      gSystem->otaUpdate.state = 0; // von check (-1) auf ruhend (0) wechseln
    // kein Speichern im EE, Zustand -1 ist nur temporär
  }
}
// GET/POST Generator
String Cloud::createCommand(bool meth, int para, const char *link, const char *host, int content)
{

  String command;
  command += meth ? F("POST ") : F("GET ");
  command += String(link);
  command += (para != NOPARA) ? "?" : "";

  switch (para)
  {

  case TESTPARA:
    break;

  default:
    break;
  }

  command += F(" HTTP/1.1\n");

  if (content > 0)
  {
    command += F("Content-Type: application/json\n");
    command += F("Content-Length: ");
    command += String(content);
    command += F("\n");
  }

  command += F("User-Agent: WLANThermo nano\n");
  command += F("SN: ");
  command += gSystem->getSerialNumber();
  command += F("\n");
  command += F("Host: ");
  command += String(host);
  command += F("\n\n");

  return command;
}