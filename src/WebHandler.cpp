/*************************************************** 
    Copyright (C) 2016  Steffen Ochs

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
    
	  Inspired and partly taken over by Vitotai
	  https://github.com/vitotai/BrewPiLess

    HISTORY: Please refer Github History
    
 ****************************************************/
// Eventuell andere Lösung zum Auslesen des Body-Inhalts
// https://github.com/me-no-dev/ESPAsyncWebServer/issues/123
// https://github.com/me-no-dev/ESPAsyncWebServer#request-variables

#include "WebHandler.h"
#include "WServer.h"
#include "API.h"
#include "DbgPrint.h"
#include "system/SystemBase.h"
#include "display/DisplayBase.h"
#include "Version.h"
#include "RecoveryMode.h"
#include "LogRingBuffer.h"
#include "DeviceId.h"
#include <SPIFFS.h>
#include <AsyncJson.h>
#include "webui/restart.html.gz.h"

#define BAD_PATH "BAD PATH"

#define APPLICATIONJSON "application/json"

typedef void (NanoWebHandler::*ArRequestHandlerFunc)(AsyncWebServerRequest *);
typedef bool (NanoWebHandler::*ArBodyHandlerFunc)(AsyncWebServerRequest *, uint8_t *);

typedef struct NanoWebHandlerList
{
  const char *requestUrl;
  int32_t requestMethod;
  int32_t authRequestMethod;
  ArRequestHandlerFunc requestHandlerFunc;
  ArBodyHandlerFunc bodyHandlerFunc;
} NanoWebHandlerListType;

const char *public_list[] = {
    "/nano.ttf",
    "/language.js"};

void printRequest(uint8_t *datas)
{
  DPRINTF("[REQUEST]\t%s\r\n", (const char *)datas);
}

uint16_t getDC(uint16_t impuls)
{
  // impuls = value * 10  // 1.Nachkommastelle
  float val = ((float)(impuls - SERVOPULSMIN * 10) / (SERVOPULSMAX - SERVOPULSMIN)) * 100;
  return (val < 500) ? ceil(val) : floor(val); // nach oben : nach unten
}

NanoWebHandler nanoWebHandler;

static const NanoWebHandlerListType nanoWebHandlerList[] = {
    // Request handler
    {"/settings", HTTP_GET | HTTP_POST, 0, &NanoWebHandler::handleSettings, NULL},
    {"/data", HTTP_GET | HTTP_POST, 0, &NanoWebHandler::handleData, NULL},
    {"/networkscan", HTTP_GET | HTTP_POST, 0, &NanoWebHandler::handleWifiScan, NULL},
    {"/networklist", HTTP_GET | HTTP_POST, 0, &NanoWebHandler::handleWifiResult, NULL},
    {"/configreset", HTTP_GET | HTTP_POST, 0, &NanoWebHandler::handleConfigReset, NULL},
    {"/stopwifi", HTTP_GET | HTTP_POST, 0, &NanoWebHandler::handleStopWifi, NULL},
    {"/checkupdate", HTTP_GET | HTTP_POST, 0, &NanoWebHandler::handleCheckUpdate, NULL},
    {"/updatestatus", HTTP_POST, 0, &NanoWebHandler::handleUpdateStatus, NULL},
    {"/dcstatus", HTTP_POST, 0, &NanoWebHandler::handleDcStatus, NULL},
    {"/clearwifi", HTTP_GET | HTTP_POST, 0, &NanoWebHandler::handleClearWifi, NULL},
    {"/recovery", HTTP_GET, HTTP_GET, &NanoWebHandler::handleRecovery, NULL},
    {"/rotate", HTTP_POST, HTTP_POST, &NanoWebHandler::handleRotate, NULL},
    {"/calibrate", HTTP_POST, HTTP_POST, &NanoWebHandler::handleCalibrate, NULL},
    {"/setadmin", HTTP_GET | HTTP_POST, HTTP_POST, &NanoWebHandler::handleAdmin, NULL},
    {"/update", HTTP_GET | HTTP_POST, HTTP_POST, &NanoWebHandler::handleUpdate, NULL},
    {"/getbluetooth", HTTP_GET | HTTP_POST, 0, &NanoWebHandler::handleBluetooth, NULL},
    {"/getdeviceid", HTTP_GET | HTTP_POST, 0, &NanoWebHandler::handleDeviceId, NULL},
    {"/log", HTTP_GET | HTTP_POST, HTTP_GET | HTTP_POST, &NanoWebHandler::handleLog, NULL},
    {"/getpush", HTTP_GET | HTTP_POST, 0, &NanoWebHandler::handleGetPush, NULL},
    // Body handler
    {"/setnetwork", HTTP_POST, 0, NULL, &NanoWebHandler::setNetwork},
    {"/setchannels", HTTP_POST, HTTP_POST, NULL, &NanoWebHandler::setChannels},
    {"/setsystem", HTTP_POST, HTTP_POST, NULL, &NanoWebHandler::setSystem},
    {"/setpitmaster", HTTP_POST, HTTP_POST, NULL, &NanoWebHandler::setPitmaster},
    {"/setpid", HTTP_POST, HTTP_POST, NULL, &NanoWebHandler::setPID},
    {"/setIoT", HTTP_POST, HTTP_POST, NULL, &NanoWebHandler::setIoT},
    {"/setpush", HTTP_POST, HTTP_POST, NULL, &NanoWebHandler::setPush},
    {"/setapi", HTTP_POST, HTTP_POST, NULL, &NanoWebHandler::setServerAPI},
    {"/setDC", HTTP_POST, 0, NULL, &NanoWebHandler::setDCTest},
    {"/setbluetooth", HTTP_POST, HTTP_POST, NULL, &NanoWebHandler::setBluetooth}};

NanoWebHandler::NanoWebHandler()
{
}

void NanoWebHandler::handleRequest(AsyncWebServerRequest *request)
{
  for (uint8_t i = 0u; i < sizeof(nanoWebHandlerList) / sizeof(NanoWebHandlerList); i++)
  {
    if (NULL == nanoWebHandlerList[i].requestHandlerFunc)
      continue;

    if (request->url().equals(nanoWebHandlerList[i].requestUrl))
    {
      if ((request->method() & nanoWebHandlerList[i].requestMethod) > 0u)
      {
        if (((request->method() & nanoWebHandlerList[i].authRequestMethod) > 0u) && (true == WServer::requireAuth()))
        {
          if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str(), WServer::getRealm()))
          {
            request->requestAuthentication(WServer::getRealm());
            break;
          }
        }

        (this->*nanoWebHandlerList[i].requestHandlerFunc)(request);
        break;
      }
      else
      {
        request->send(500, TEXTPLAIN, BAD_PATH);
      }
    }
  }
}

void NanoWebHandler::handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  for (uint8_t i = 0u; i < sizeof(nanoWebHandlerList) / sizeof(NanoWebHandlerList); i++)
  {
    if (NULL == nanoWebHandlerList[i].bodyHandlerFunc)
      continue;

    if (request->url().equals(nanoWebHandlerList[i].requestUrl))
    {
      if ((request->method() & nanoWebHandlerList[i].requestMethod) > 0u)
      {
        if (((request->method() & nanoWebHandlerList[i].authRequestMethod) > 0u) && (true == WServer::requireAuth()))
        {
          if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str(), WServer::getRealm()))
          {
            request->requestAuthentication(WServer::getRealm());
            break;
          }
        }

        if ((this->*nanoWebHandlerList[i].bodyHandlerFunc)(request, data))
        {
          request->send(200, TEXTPLAIN, TEXTTRUE);
        }
        else
        {
          request->send(200, TEXTPLAIN, TEXTFALSE);
        }

        break;
      }
      else
      {
        request->send(500, TEXTPLAIN, BAD_PATH);
      }
    }
  }
}

bool NanoWebHandler::canHandle(AsyncWebServerRequest *request)
{
  boolean supported = false;

  for (uint8_t i = 0u; i < sizeof(nanoWebHandlerList) / sizeof(NanoWebHandlerList); i++)
  {
    if ((request->url().equals(nanoWebHandlerList[i].requestUrl)) && ((request->method() & nanoWebHandlerList[i].requestMethod) > 0u))
    {
      supported = true;
      break;
    }
  }

  return supported;
}

void NanoWebHandler::handleSettings(AsyncWebServerRequest *request)
{

  String jsonStr;
  jsonStr = API::apiData(APISETTINGS);

  request->send(200, APPLICATIONJSON, jsonStr);
}

void NanoWebHandler::handleData(AsyncWebServerRequest *request)
{

  String jsonStr;
  jsonStr = API::apiData(APIDATA);

  request->send(200, APPLICATIONJSON, jsonStr);
}

void NanoWebHandler::handleWifiResult(AsyncWebServerRequest *request)
{
  AsyncJsonResponse *response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");

  JsonObject &json = response->getRoot();

  int n = WiFi.scanComplete();

  if (n > 0)
  {

    if (WiFi.status() == WL_CONNECTED)
    {
      json["Connect"] = true;
      json["SSID"] = WiFi.SSID();
      json["BSSID"] = WiFi.BSSIDstr();
      json["IP"] = WiFi.localIP().toString();
      json["Mask"] = WiFi.subnetMask().toString();
      json["Gate"] = WiFi.gatewayIP().toString();
    }
    else
    {
      json["Connect"] = false;
      json["SSID"] = gSystem->wlan.getAccessPointName();
      json["IP"] = WiFi.softAPIP().toString();
    }

    String filterDuplicates;
    JsonArray &_scan = json.createNestedArray("Scan");
    for (uint8_t i = 0; i < n; i++)
    {
      if (filterDuplicates.indexOf("||" + WiFi.SSID(i) + "||") >= 0)
        continue;

      filterDuplicates += "||" + WiFi.SSID(i) + "||";
      JsonObject &_wifi = _scan.createNestedObject();
      _wifi["SSID"] = WiFi.SSID(i);
      _wifi["BSSID"] = WiFi.BSSIDstr(i);
      _wifi["RSSI"] = WiFi.RSSI(i);
      _wifi["Enc"] = WiFi.encryptionType(i);
      if ((WiFi.status() == WL_CONNECTED) && (WiFi.SSID(i) == WiFi.SSID()) && (WiFi.BSSIDstr(i) == WiFi.BSSIDstr()))
      {
        json["RSSI"] = WiFi.RSSI();
        json["Enc"] = WiFi.encryptionType(i);
      }
    }
  }

  if (request)
  {
    response->setLength();
    request->send(response);
  }
  else
    json.printTo(Serial);
}

void NanoWebHandler::handleWifiScan(AsyncWebServerRequest *request)
{

  // kein Scan zu Systemstart sonst keine Reconnection nach Systemstart

  WiFi.scanDelete();
  if (WiFi.scanComplete() == -2)
  {
    WiFi.scanNetworks(true); // true = scan async

    if (request)
      request->send(200, TEXTPLAIN, "OK");
    //else Serial.println("OK");
  }
}

void NanoWebHandler::handleConfigReset(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_GET)
  {
    request->send(200, "text/html", "<form method='POST' action='/configreset'>System-Speicher wirklich resetten?<br><br><input type='submit' value='Ja'></form>");
  }
  else if (request->method() == HTTP_POST)
  {
    gSystem->resetConfig();
    request->send(200, TEXTPLAIN, TEXTTRUE);
  }
  else
  {
    request->send(500, TEXTPLAIN, BAD_PATH);
  }
}

void NanoWebHandler::handleStopWifi(AsyncWebServerRequest *request)
{
  gSystem->wlan.setStopRequest();
  request->send(200, TEXTPLAIN, TEXTTRUE);
}

void NanoWebHandler::handleCheckUpdate(AsyncWebServerRequest *request)
{
  gSystem->otaUpdate.resetUpdateInfo();
  gSystem->otaUpdate.askUpdateInfo();
  request->send(200, TEXTPLAIN, TEXTTRUE);
}

void NanoWebHandler::handleUpdateStatus(AsyncWebServerRequest *request)
{
  DPRINTLN("... in process");
  if (gSystem->otaUpdate.isUpdateInProgress())
  {
    request->send(200, TEXTPLAIN, TEXTTRUE);
  }
  else
  {
    request->send(200, TEXTPLAIN, TEXTFALSE);
  }
}

void NanoWebHandler::handleDcStatus(AsyncWebServerRequest *request)
{
  boolean dctRunning = false;
  for (uint8_t i = 0u; i < gSystem->pitmasters.count(); i++)
  {
    Pitmaster *pm = gSystem->pitmasters[i];

    if (pm != NULL)
      if (true == pm->isDutyCycleTestRunning())
        dctRunning = true;
  }
  if (true == dctRunning)
    request->send(200, TEXTPLAIN, TEXTTRUE);
  else
    request->send(200, TEXTPLAIN, TEXTFALSE);
}

void NanoWebHandler::handleClearWifi(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_GET)
  {
    request->send(200, "text/html", "<form method='POST' action='/clearwifi'>Wifi-Speicher wirklich leeren?<br><br><input type='submit' value='Ja'></form>");
  }
  else if (request->method() == HTTP_POST)
  {
    request->send(200, TEXTPLAIN, TEXTTRUE);
    gSystem->wlan.clearCredentials();
    gSystem->restart();
  }
}

void NanoWebHandler::handleRecovery(AsyncWebServerRequest *request)
{
  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", restart_html_gz, sizeof(restart_html_gz));
  response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);

  WlanCredentials credentials;
  gSystem->wlan.getCredentials(&credentials);
  RecoveryMode::runFromApp(credentials.ssid, credentials.password);
}

void NanoWebHandler::handleRotate(AsyncWebServerRequest *request)
{
  gDisplay->toggleOrientation();
  gDisplay->saveConfig();
  if (gDisplay->getUpdateName() != "")
  {
    gSystem->otaUpdate.resetUpdateInfo();
    gSystem->otaUpdate.setForceFlag(true);
    gSystem->otaUpdate.requestFile(gDisplay->getUpdateName());
    gSystem->otaUpdate.requestVersion(FIRMWAREVERSION);
    gSystem->otaUpdate.askUpdateInfo();
  }

  AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", restart_html_gz, sizeof(restart_html_gz));
  response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

void NanoWebHandler::handleCalibrate(AsyncWebServerRequest *request)
{
  gDisplay->calibrate();
  request->send(200, TEXTPLAIN, TEXTTRUE);
}

void NanoWebHandler::handleAdmin(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_GET)
  {
    request->send(200, "text/html", "<form method='POST' action='/setadmin'>Neues Password eingeben (max. 10 Zeichen): <input type='text' name='wwwpw'><br><br><input type='submit' value='Change'></form>");
  }
  else if (request->method() == HTTP_POST)
  {
    if (request->hasParam("wwwpw", true))
    {
      String password = request->getParam("wwwpw", true)->value();
      if (password.length() < 11)
      {
        WServer::setPassword(password);
        WServer::saveConfig();
        request->send(200, TEXTPLAIN, TEXTTRUE);
      }
      else
      {
        request->send(200, TEXTPLAIN, TEXTFALSE);
      }
    }
  }
}

void NanoWebHandler::handleUpdate(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_GET)
  {
    request->send(200, "text/html", "<form method='POST' action='/update'>Version mit v eingeben: <input type='text' name='version'><br><br><input type='submit' value='Update'></form>");
  }
  else if (request->method() == HTTP_POST)
  {
    if (request->hasParam("version", true))
    {
      String version = request->getParam("version", true)->value();
      Serial.println(version);
      if (version.indexOf("v") == 0)
      {
        gSystem->otaUpdate.resetUpdateInfo();
        gSystem->otaUpdate.setForceFlag(true);
        gSystem->otaUpdate.requestVersion(version);
        gSystem->otaUpdate.askUpdateInfo();
      }
      else
        request->send(200, TEXTPLAIN, "Version unknown!");
    }
    else
    {
      gSystem->otaUpdate.startUpdate();
    }
    request->send(200, TEXTPLAIN, "Do Update...");
  }
}

void NanoWebHandler::handleBluetooth(AsyncWebServerRequest *request)
{
  AsyncJsonResponse *response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");

  JsonObject &json = response->getRoot();
  json["enabled"] = gSystem->bluetooth->isEnabled();
  JsonArray &_devices = json.createNestedArray("devices");

  for (uint8_t deviceIndex = 0u; deviceIndex < gSystem->bluetooth->getDeviceCount(); deviceIndex++)
  {
    BleDeviceType bleDevice;
    if (gSystem->bluetooth->getDevice(deviceIndex, &bleDevice))
    {
      if (bleDevice.count > 0u)
      {
        JsonObject &_device = _devices.createNestedObject();
        _device["name"] = String(bleDevice.name);
        _device["address"] = String(bleDevice.address);
        _device["count"] = bleDevice.count;
        _device["selected"] = bleDevice.selected;
      }
    }
  }

  response->setLength();
  request->send(response);
}

void NanoWebHandler::handleDeviceId(AsyncWebServerRequest *request)
{
  request->send(200, TEXTPLAIN, DeviceId::get());
}

void NanoWebHandler::handleLog(AsyncWebServerRequest *request)
{
  request->send(200, TEXTPLAIN, gLogRingBuffer.get());
}

void NanoWebHandler::handleGetPush(AsyncWebServerRequest *request)
{
  AsyncJsonResponse *response = new AsyncJsonResponse();
  response->addHeader("Server", "ESP Async Web Server");

  JsonObject &json = response->getRoot();

  PushTelegramType pushTelegram = gSystem->notification.getTelegramConfig();
  PushPushoverType pushPushover = gSystem->notification.getPushoverConfig();
  PushAppType pushApp = gSystem->notification.getAppConfig();

  JsonObject &telegram = json.createNestedObject("telegram");

  telegram["enabled"] = pushTelegram.enabled;
  telegram["token"] = pushTelegram.token;
  telegram["chat_id"] = pushTelegram.chatId;

  JsonObject &pushover = json.createNestedObject("pushover");

  pushover["enabled"] = pushPushover.enabled;
  pushover["token"] = pushPushover.token;
  pushover["user_key"] = pushPushover.userKey;
  pushover["priority"] = pushPushover.priority;

  JsonObject &app = json.createNestedObject("app");

  app["enabled"] = pushApp.enabled;
  app["max_devices"] = PUSH_APP_MAX_DEVICES;

  JsonArray &devices = app.createNestedArray("devices");

  for (uint8_t i = 0u; i < PUSH_APP_MAX_DEVICES; i++)
  {
    if (strlen(pushApp.devices[i].token) > 0u)
    {
      JsonObject &device = devices.createNestedObject();

      device["name"] = pushApp.devices[i].name;
      device["id"] = pushApp.devices[i].id;
      device["token_sha256"] = Notification::getTokenSha256(pushApp.devices[i].token);
      device["sound"] = pushApp.devices[i].sound;
    }
  }

  response->setLength();
  request->send(response);
}

int NanoWebHandler::checkStringLength(String tex)
{
  int index = tex.length();
  while (tex.lastIndexOf(195) != -1)
  {
    tex.remove(tex.lastIndexOf(195), 1);
    index--;
  }
  return index;
}

String NanoWebHandler::checkString(String tex)
{
  tex.replace("&amp;", "&"); // &
  tex.replace("&lt;", "<");  // <
  tex.replace("&gt;", ">");  // >
  return tex;
}

bool NanoWebHandler::setSystem(AsyncWebServerRequest *request, uint8_t *datas)
{

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &_system = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!_system.success())
    return 0;

  String unit, _name;

  if (_system.containsKey("language"))
    gSystem->setLanguage(_system["language"].asString());
  if (_system.containsKey("crashreport"))
    gSystem->setCrashReport((boolean)_system["crashreport"]);

  gSystem->saveConfig();

  if (_system.containsKey("unit"))
    unit = _system["unit"].asString();
  if (_system.containsKey("autoupd"))
    gSystem->otaUpdate.setAutoUpdate((boolean)_system["autoupd"]);
  if (_system.containsKey("prerelease"))
    gSystem->otaUpdate.setPrerelease(_system["prerelease"]);

  gSystem->otaUpdate.saveConfig();

  if (_system.containsKey("host"))
  {
    _name = _system["host"].asString();
    if (checkStringLength(_name) < 14)
      gSystem->wlan.setHostName(_name);
  }

  if (_system.containsKey("ap"))
  {
    _name = _system["ap"].asString();
    if (checkStringLength(_name) < 14)
      gSystem->wlan.setAccessPointName(_name);
  }

  gSystem->wlan.saveConfig();

  gSystem->temperatures.setUnit((TemperatureUnit)unit.charAt(0));
  gSystem->temperatures.saveConfig();

  return 1;
}

bool NanoWebHandler::setChannels(AsyncWebServerRequest *request, uint8_t *datas)
{

  //  https://github.com/me-no-dev/ESPAsyncWebServer/issues/123

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &_cha = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!_cha.success())
    return 0;

  int num = _cha["number"];
  num--; // Intern beginnt die Zählung bei 0

  TemperatureBase *temperature = gSystem->temperatures[num];

  if (temperature != NULL)
  { // Feld vorhanden
    String _name;
    if (_cha.containsKey("name"))
    {
      _name = _cha["name"].asString(); // KANALNAME
      if (checkStringLength(_name) < 11)
        temperature->setName(_name.c_str());
    }

    byte _typ;
    if (_cha.containsKey("typ"))
    {
      _typ = _cha["typ"]; // FÜHLERTYP
      if (_typ > -1 && _typ < temperature->getTypeCount())
        temperature->setType(_typ);
    }

    float _limit;
    if (_cha.containsKey("min"))
    {
      _limit = _cha["min"]; // LIMIT
      temperature->setMinValue(_limit);
    }

    if (_cha.containsKey("max"))
    {
      _limit = _cha["max"]; // LIMIT
      temperature->setMaxValue(_limit);
    }

    if (_cha.containsKey("alarm"))
      temperature->setAlarmSetting((AlarmSetting)_cha["alarm"].as<uint8_t>()); // ALARM
    if (_cha.containsKey("color"))
      temperature->setColor(_cha["color"].asString()); // COLOR
  }
  else
    return 0;

  gSystem->temperatures.saveConfig();
  return 1;
}

bool NanoWebHandler::setNetwork(AsyncWebServerRequest *request, uint8_t *datas)
{

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  String ssid;
  String password;
  JsonObject &_network = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!_network.success())
    return 0;

  if (_network.containsKey("ssid"))
  {
    ssid = checkString(_network["ssid"].asString());

    if (_network.containsKey("password"))
      password = _network["password"].asString();

    gSystem->wlan.addCredentials(ssid.c_str(), password.c_str());

    return 1;
  }

  return 0;
}

bool NanoWebHandler::addNetwork(AsyncWebServerRequest *request, uint8_t *datas)
{

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  String ssid;
  String password;
  JsonObject &_network = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!_network.success())
    return 0;

  if (_network.containsKey("ssid"))
  {
    ssid = checkString(_network["ssid"].asString());

    if (_network.containsKey("password"))
      password = _network["password"].asString();

    gSystem->wlan.addCredentials(ssid.c_str(), password.c_str(), true);

    return 1;
  }

  return 0;
}

bool NanoWebHandler::setIoT(AsyncWebServerRequest *request, uint8_t *datas)
{

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &_chart = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!_chart.success())
    return 0;

  MqttConfig mqttConfig = gSystem->mqtt.getConfig();
  CloudConfig cloudConfig = gSystem->cloud.getConfig();
  bool refresh = cloudConfig.cloudEnabled;

  if (_chart.containsKey("PMQhost"))
    strcpy(mqttConfig.host, _chart["PMQhost"].asString());
  if (_chart.containsKey("PMQport"))
    mqttConfig.port = _chart["PMQport"];
  if (_chart.containsKey("PMQuser"))
    strcpy(mqttConfig.user, _chart["PMQuser"].asString());
  if (_chart.containsKey("PMQpass"))
    strcpy(mqttConfig.password, _chart["PMQpass"].asString());
  if (_chart.containsKey("PMQqos"))
    mqttConfig.QoS = _chart["PMQqos"];
  if (_chart.containsKey("PMQon"))
    mqttConfig.enabled = _chart["PMQon"];
  if (_chart.containsKey("PMQint"))
    mqttConfig.interval = _chart["PMQint"];

  gSystem->mqtt.setConfig(mqttConfig);

  if (_chart.containsKey("CLon"))
    cloudConfig.cloudEnabled = _chart["CLon"];
  if (_chart.containsKey("CLtoken"))
    cloudConfig.cloudToken = _chart["CLtoken"].asString();
  if (_chart.containsKey("CLint"))
    cloudConfig.cloudInterval = _chart["CLint"];
  
  if (_chart.containsKey("CCLon"))
    cloudConfig.customEnabled = _chart["CCLon"];
  if (_chart.containsKey("CCLurl"))
    cloudConfig.customUrl = _chart["CCLurl"].asString();
  if (_chart.containsKey("CCLint"))
    cloudConfig.customInterval = _chart["CCLint"];

  gSystem->cloud.setConfig(cloudConfig);

  return 1;
}

bool NanoWebHandler::setPush(AsyncWebServerRequest *request, uint8_t *datas)
{

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &_push = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!_push.success())
    return 0;

  boolean sendTestMessage = false;

  // check if test message is requested
  if (_push.containsKey("test"))
  {
    if (true == _push["test"].as<boolean>())
    {
      sendTestMessage = true;
    }
  }

  if (_push.containsKey("telegram"))
  {
    JsonObject &_telegram = _push["telegram"];

    if (_telegram.containsKey("enabled") && _telegram.containsKey("token") &&
        _telegram.containsKey("chat_id"))
    {
      // set telegram
      PushTelegramType telegram;
      memset(&telegram, 0, sizeof(telegram));

      telegram.enabled = _telegram["enabled"];
      strncpy(telegram.token, _telegram["token"].asString(), sizeof(telegram.token));
      strncpy(telegram.chatId, _telegram["chat_id"].asString(), sizeof(telegram.chatId));
      gSystem->notification.setTelegramConfig(telegram, sendTestMessage);
    }
  }

  if (_push.containsKey("pushover"))
  {
    JsonObject &_pushover = _push["pushover"];

    if (_pushover.containsKey("enabled") && _pushover.containsKey("token") &&
        _pushover.containsKey("user_key") && _pushover.containsKey("priority"))
    {
      // set pushover
      PushPushoverType pushover;
      memset(&pushover, 0, sizeof(pushover));
      pushover.enabled = _pushover["enabled"];
      strncpy(pushover.token, _pushover["token"].asString(), sizeof(pushover.token));
      strncpy(pushover.userKey, _pushover["user_key"].asString(), sizeof(pushover.userKey));
      pushover.priority = _pushover["priority"];

      if (_pushover.containsKey("retry") && _pushover.containsKey("expire"))
      {
        pushover.retry = _pushover["retry"];
        pushover.expire = _pushover["expire"];
      }

      gSystem->notification.setPushoverConfig(pushover, sendTestMessage);
    }
  }

  if (_push.containsKey("app"))
  {
    JsonObject &_app = _push["app"];

    if (_app.containsKey("enabled") && _app.containsKey("devices"))
    {
      // set app
      PushAppType app;
      memset(&app, 0, sizeof(app));

      app.enabled = _app["enabled"];

      JsonArray &_devices = _app["devices"];
      uint8_t deviceIndex = 0u;

      for (JsonArray::iterator it = _devices.begin(); (it != _devices.end()) && (deviceIndex < PUSH_APP_MAX_DEVICES); ++it)
      {
        JsonObject &_device = it->asObject();

        if (_device.containsKey("name") && _device.containsKey("id") &&
            (_device.containsKey("token") || _device.containsKey("token_sha256")))
        {
          String token;
          boolean hasToken = _device.containsKey("token");
          boolean hasHashedToken = _device.containsKey("token_sha256");
          boolean hasSound = _device.containsKey("sound");

          // check length of name and id
          if ((strlen(_device["name"].asString()) >= sizeof(PushAppDeviceType::name)) &&
              (strlen(_device["id"].asString()) >= sizeof(PushAppDeviceType::id)))
          {
            continue;
          }

          // check length of token
          if(hasToken)
          {
            if(strlen(_device["token"].asString()) >= sizeof(PushAppDeviceType::token))
            {
              continue;
            }
          }

          strcpy(app.devices[deviceIndex].name, _device["name"].asString());
          strcpy(app.devices[deviceIndex].id, _device["id"].asString());

          // user token or hash to get token
          if (hasToken)
          {
            token = _device["token"].asString();
          }
          else if (hasHashedToken)
          {
            token = gSystem->notification.getDeviceTokenFromHash(_device["token_sha256"].asString());
          }

          strcpy(app.devices[deviceIndex].token, token.c_str());

          if(hasSound)
          {
            app.devices[deviceIndex].sound = _device["sound"].as<uint8_t>();
          }

          deviceIndex++;
        }
      }

      gSystem->notification.setAppConfig(app, sendTestMessage);
    }
  }

  if (false == sendTestMessage)
  {
    gSystem->notification.saveConfig();
  }

  return 1;
}

bool NanoWebHandler::setPitmaster(AsyncWebServerRequest *request, uint8_t *datas)
{

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonArray &json = jsonBuffer.parseArray((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!json.success())
    return 0;

  byte id, ii = 0;

  for (JsonArray::iterator it = json.begin(); it != json.end(); ++it)
  {

    JsonObject &_pitmaster = json[ii];

    Pitmaster *pm = gSystem->pitmasters[ii];

    if (NULL == pm)
      continue;

    if (_pitmaster.containsKey("id"))
      id = _pitmaster["id"];
    else
      break;
    if (id >= gSystem->pitmasters.count())
      break;

    String typ;
    if (_pitmaster.containsKey("typ"))
      typ = _pitmaster["typ"].asString();
    else
      return 0;

    if (_pitmaster.containsKey("channel"))
    {
      byte cha = _pitmaster["channel"];
      pm->assignTemperature(gSystem->temperatures[cha - 1]);

      //open_lid_init(); // Speicher zurücksetzen
      //TODO: clear open lid data at pitmaster
    }
    else
      return 0;

    if (_pitmaster.containsKey("pid"))
    {
      byte temppid = _pitmaster["pid"];
      if (temppid != pm->getAssignedProfile()->id)
      {
        pm->disableActuators(false);
        //Serial.println("PID-Wechsel");
      }
      pm->assignProfile(gSystem->getPitmasterProfile(temppid));
    }
    else
      return 0;
    if (_pitmaster.containsKey("set"))
      pm->setTargetTemperature(_pitmaster["set"]);
    else
      return 0;

    bool _manual = false;
    bool _auto = false;

    if (typ == "auto")
      _auto = true;
    else if (typ == "manual")
      _manual = true;
    else
      pm->setType(pm_off);

    if (_pitmaster.containsKey("value") && _manual)
    {
      int _val = _pitmaster["value"];
      pm->setValue(constrain(_val, 0, 100));
      pm->setType(pm_manual);
    }

    // kann gespeichert werden und was ist mit stoppen des autotune
    if (_auto)
    {
      pm->setType(pm_auto);
      if (pm->getAssignedProfile()->autotune)
        pm->startAutoTune();
    }

    ii++;
  }

  gSystem->pitmasters.saveConfig();

  return 1;
}

bool NanoWebHandler::setPID(AsyncWebServerRequest *request, uint8_t *datas)
{

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonArray &json = jsonBuffer.parseArray((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!json.success())
    return 0;

  byte id = 0, ii = 0;
  float val;

  for (JsonArray::iterator it = json.begin(); it != json.end(); ++it)
  {
    JsonObject &_pid = json[ii];
    if (_pid.containsKey("id"))
      id = _pid["id"];
    else
      break;
    if (id >= gSystem->getPitmasterProfileCount())
      break;

    PitmasterProfile *profile = gSystem->getPitmasterProfile(id);

    if (_pid.containsKey("name"))
      profile->name = _pid["name"].asString();
    if (_pid.containsKey("aktor"))
      profile->actuator = _pid["aktor"];
    if (_pid.containsKey("Kp"))
      profile->kp = _pid["Kp"];
    if (_pid.containsKey("Ki"))
      profile->ki = _pid["Ki"];
    if (_pid.containsKey("Kd"))
      profile->kd = _pid["Kd"];
    if (_pid.containsKey("DCmmin"))
    {
      val = _pid["DCmmin"];
      profile->dcmin = constrain(val * 10, 0, 1000) / 10.0; // 1. Nachkommastelle
    }
    if (_pid.containsKey("DCmmax"))
    {
      val = _pid["DCmmax"];
      profile->dcmax = constrain(val * 10, 0, 1000) / 10.0; // 1. Nachkommastelle
    }
    if (_pid.containsKey("opl"))
      profile->opl = _pid["opl"];
    if (_pid.containsKey("tune"))
      profile->autotune = _pid["tune"];
    if (_pid.containsKey("jp"))
      profile->jumppw = constrain(_pid["jp"], 10, 100);
    if (_pid.containsKey("SPmin"))
    {
      val = _pid["SPmin"];
      if (val >= SERVOPULSMIN && val <= SERVOPULSMAX && (profile->actuator == SERVO || profile->actuator == DAMPER))
      {
        profile->spmin = getDC(val * 10) / 10.0;
      }
      else
        profile->spmin = constrain(val * 10, 0, 1000) / 10.0; // 1. Nachkommastelle
    }
    if (_pid.containsKey("SPmax"))
    {
      val = _pid["SPmax"];
      if (val >= SERVOPULSMIN && val <= SERVOPULSMAX && (profile->actuator == SERVO || profile->actuator == DAMPER))
      {
        profile->spmax = getDC(val * 10) / 10.0;
      }
      else
        profile->spmax = constrain(val * 10, 0, 1000) / 10.0; // 1. Nachkommastelle
    }
    if (_pid.containsKey("link"))
      profile->link = _pid["link"];

    ii++;
  }

  gSystem->pitmasters.saveConfig();

  return 1;
}

bool NanoWebHandler::setServerAPI(AsyncWebServerRequest *request, uint8_t *datas)
{

  //printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!json.success())
    return 0;

  // URL
  if (json.containsKey("url"))
  {
    Serial.println("Server-URL");
    JsonObject &_url = json["url"];

    for (int i = 0; i < Cloud::serverurlCount; i++)
    { // nur bekannte auslesen
      JsonObject &_link = _url[Cloud::serverurl[i].typ];
      if (_link.containsKey("host"))
        Cloud::serverurl[i].host = _link["host"].asString();
      if (_link.containsKey("page"))
        Cloud::serverurl[i].page = _link["page"].asString();
    }

    gSystem->cloud.saveUrl();
  }

  // UPDATE
  bool available = false;
  if (json.containsKey("update"))
  {
    Serial.println("Update object");
    JsonObject &_update = json["update"];
    if (_update.containsKey("available"))
      available = _update["available"];

    if (available)
    {
      // bei gSystem->otaUpdate.get wurde eine bestimmte Version angefragt
      String version;
      if (_update.containsKey("version"))
      {
        version = _update["version"].asString();

        if (!gSystem->otaUpdate.checkForUpdate(version))
        {
          // keine konrekte Anfrage
          gSystem->otaUpdate.setUpdateVersion(version);
        }
      }

      if (_update.containsKey(gDisplay->getUpdateName()))
      { // Firmware-Link
        JsonObject &_fw = _update[gDisplay->getUpdateName()];
        if (_fw.containsKey("url"))
          gSystem->otaUpdate.setDisplayUrl(_fw["url"].asString());
        Serial.println(_fw["url"].asString());
      }

      if (_update.containsKey("firmware"))
      { // Firmware-Link
        JsonObject &_fw = _update["firmware"];
        if (_fw.containsKey("url"))
          gSystem->otaUpdate.setFirmwareUrl(_fw["url"].asString());
        Serial.println(_fw["url"].asString());
      }

      if (_update.containsKey("force"))
      {
        gSystem->otaUpdate.startUpdate();
      }
    }
    else
    {
      gSystem->otaUpdate.setUpdateVersion("false");
    }
  }

  // CLOUD
  if (json.containsKey("cloud"))
  {
    JsonObject &_cloud = json["cloud"];

    if (_cloud.containsKey("task"))
    {
      if (_cloud["task"])
        gSystem->cloud.state = 2;
      else
        gSystem->cloud.state = 1;
      Serial.print("[CLOUD]: ");
      Serial.println(gSystem->cloud.state);
    }
  }

  // NOTE
  if (json.containsKey("notification"))
  {
    JsonObject &_note = json["notification"];

    if (_note.containsKey("task"))
    {
      //if (_note["task"]) sys.online |= (1<<2);
      //else sys.online &= ~(1<<2);
      Serial.print("[NOTE]: ");
      Serial.println(_note["task"].asString());
    }
  }

  return 1;
}

bool NanoWebHandler::setDCTest(AsyncWebServerRequest *request, uint8_t *datas)
{

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!json.success())
    return 0;

  byte aktor = json["aktor"];
  bool dc = json["dc"];
  int val = json["val"];
  val /= 10;                                                      //TODO: why is value multiplied with 10 in frontend?
  byte id = 0;                                                    // Pitmaster0 // TODO: add id to frontend
  return gSystem->pitmasters[id]->startDutyCycleTest(aktor, val); //TODO NULL pointer!!!
}

bool NanoWebHandler::setBluetooth(AsyncWebServerRequest *request, uint8_t *datas)
{
  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!json.success())
    return 0;

  if (json.containsKey("enabled"))
  {
    gSystem->bluetooth->enable(json["enabled"].as<boolean>());
  }

  JsonArray &_devices = json["devices"].asArray();

  for (JsonArray::iterator itDevice = _devices.begin(); itDevice != _devices.end(); ++itDevice)
  {
    JsonObject &_device = itDevice->asObject();

    if (_device.containsKey("address") == false || _device.containsKey("count") == false || _device.containsKey("selected") == false)
    {
      Serial.println("Invalid JSON!");
      continue;
    }

    uint8_t count = _device["count"];
    uint32_t selected = _device["selected"];

    for (uint8_t i = 0u; i < count; i++)
    {
      if (selected & (1 << i))
      {
        gSystem->temperatures.add((uint8_t)SensorType::Ble, _device["address"], i);
      }
      else
      {
        gSystem->temperatures.remove((uint8_t)SensorType::Ble, _device["address"], i);
      }
    }

    gSystem->bluetooth->setDeviceSelected(_device["address"], selected);
  }

  gSystem->temperatures.saveConfig();
  gSystem->bluetooth->saveConfig();
}