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
#include "Server.h"
#include "API.h"
#include "DbgPrint.h"
#include "system/SystemBase.h"
#include "Version.h"
#include "RecoveryMode.h"
#include <SPIFFS.h>
#include <AsyncJson.h>
#include "webui/restart.html.gz.h"

#define FLIST_PATH "/list"
#define DELETE_PATH "/rm"
#define FPUTS_PATH "/fputs"
#define DATA_PATH "/data"
#define SETTING_PATH "/settings"
#define UPDATE_PATH "/update"
#define BAD_PATH "BAD PATH"
#define DEFAULT_INDEX_FILE "index.html"
#define LOGLIST_PATH "/loglist.php"
#define CHART_DATA_PATH "/chart.php"
#define NETWORK_SCAN "/networkscan"
#define NETWORK_LIST "/networklist"
#define NETWORK_STOP "/stopwifi"
#define NETWORK_CLEAR "/clearwifi"
#define CONFIG_RESET "/configreset"
#define ADMIN "/admin"
#define UPLOAD "/upload"
#define HISTORY "/history"
#define RECOVERY "/recovery"

#define SET_NETWORK "/setnetwork"
#define SET_SYSTEM "/setsystem"
#define SET_CHANNELS "/setchannels"
#define SET_PITMASTER "/setpitmaster"
#define SET_PID "/setpid"
#define SET_DC "/setDC"
#define SET_IOT "/setIoT"
#define SET_PUSH "/setPush"
#define SET_API "/setapi"
#define SET_GOD "/god"
#define UPDATE_CHECK "/checkupdate"
#define UPDATE_STATUS "/updatestatus"
#define DC_STATUS "/dcstatus"

#define APPLICATIONJSON "application/json"

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
BodyWebHandler bodyWebHandler;

NanoWebHandler::NanoWebHandler(void)
{
}
void NanoWebHandler::handleSettings(AsyncWebServerRequest *request)
{

  String jsonStr;
  jsonStr = API::apiData(APISETTINGS);

  request->send(200, APPLICATIONJSON, jsonStr);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void NanoWebHandler::handleData(AsyncWebServerRequest *request)
{

  String jsonStr;
  jsonStr = API::apiData(APIDATA);

  request->send(200, APPLICATIONJSON, jsonStr);
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void NanoWebHandler::handleWifiResult(AsyncWebServerRequest *request)
{

  // https://github.com/me-no-dev/ESPAsyncWebServer/issues/85

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

    JsonArray &_scan = json.createNestedArray("Scan");
    for (int i = 0; i < n; i++)
    {
      JsonObject &_wifi = _scan.createNestedObject();
      _wifi["SSID"] = WiFi.SSID(i);
      _wifi["RSSI"] = WiFi.RSSI(i);
      _wifi["Enc"] = WiFi.encryptionType(i);
      if (WiFi.status() == WL_CONNECTED & WiFi.SSID(i) == WiFi.SSID())
      {
        json["Enc"] = WiFi.encryptionType(i);
        json["RSSI"] = WiFi.RSSI(i);
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

void NanoWebHandler::configreset()
{
  gSystem->resetConfig();
  gSystem->temperatures.saveConfig();
  //setconfig(eSYSTEM, {});
  //loadconfig(eSYSTEM, 0);
  // set_pitmaster(1); TODO reset pitmaster
  // set_pid(0); ; TODO reset profiles
  //set_iot(1); TODO reset iot config
  //setconfig(eTHING, {}); TODO reset cloud config
}

void NanoWebHandler::handleRequest(AsyncWebServerRequest *request)
{

  if (request->method() == HTTP_POST || request->method() == HTTP_GET)
  {

    if (request->url() == DATA_PATH)
    {
      handleData(request);
      return;
    }
    else if (request->url() == SETTING_PATH)
    {
      handleSettings(request);
      return;
    }
    else if (request->url() == NETWORK_SCAN)
    {
      handleWifiScan(request);
      return;
    }
    else if (request->url() == NETWORK_LIST)
    {
      handleWifiResult(request);
      return;

      // REQUEST: /stop wifi
    }
    else if (request->url() == NETWORK_STOP)
    {
      //TODO: stop wifi? wifi.mode = 4; // Turn Wifi off with timer
      gSystem->wlan.setStopRequest();
      request->send(200, TEXTPLAIN, TEXTTRUE);
      return;

      // REQUEST: /checkupdate
    }
    else if (request->url() == UPDATE_CHECK)
    {
      gSystem->otaUpdate.state = -1;
      request->send(200, TEXTPLAIN, TEXTTRUE);
      return;
    }
  }

  if (request->method() == HTTP_POST)
  {

    // REQUEST: /updatestatus
    if (request->url() == UPDATE_STATUS)
    {
      DPRINTLN("... in process");
      if (gSystem->otaUpdate.state > 0)
        request->send(200, TEXTPLAIN, TEXTTRUE);
      request->send(200, TEXTPLAIN, TEXTFALSE);
      return;

      // REQUEST: /dcstatus
    }
    else if (request->url() == DC_STATUS)
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
      return;
    }
  }

  // REQUEST: /clear wifi
  if (request->url() == NETWORK_CLEAR)
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
    else
      request->send(500, TEXTPLAIN, BAD_PATH);
    return;

    // REQUEST: /configreset
  }
  else if (request->url() == CONFIG_RESET)
  {
    if (request->method() == HTTP_GET)
    {
      request->send(200, "text/html", "<form method='POST' action='/configreset'>System-Speicher wirklich resetten?<br><br><input type='submit' value='Ja'></form>");
    }
    else if (request->method() == HTTP_POST)
    {
      configreset();
      request->send(200, TEXTPLAIN, TEXTTRUE);
    }
    else
      request->send(500, TEXTPLAIN, BAD_PATH);
    return;

    // REQUEST: /recovery
  }
  else if (request->url() == RECOVERY)
  {
    if (request->method() == HTTP_GET)
    {
      if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str()))
        return request->requestAuthentication();
      
      AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", restart_html_gz, sizeof(restart_html_gz));
      response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
      response->addHeader("Content-Encoding", "gzip");
      request->send(response);

      WlanCredentials credentials;
      gSystem->wlan.getCredentials(&credentials);
      RecoveryMode::runFromApp(credentials.ssid, credentials.password);
    }
    else
      request->send(500, TEXTPLAIN, BAD_PATH);
    return;

    // REQUEST: /admin
  }
  else if (request->url() == ADMIN)
  {
    if (request->method() == HTTP_GET)
    {
      request->send(200, "text/html", "<form method='POST' action='/setadmin'>Neues Password eingeben (max. 10 Zeichen): <input type='text' name='wwwpw'><br><br><input type='submit' value='Change'></form>");
    }
    else if (request->method() == HTTP_POST)
    {
      if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str()))
        return request->requestAuthentication();
      if (request->hasParam("wwwpw", true))
      {
        String password = request->getParam("wwwpw", true)->value();
        if (password.length() < 11)
        {
          WServer::setPassword(password);
          WServer::saveConfig();
          request->send(200, TEXTPLAIN, TEXTTRUE);
        }
        request->send(200, TEXTPLAIN, TEXTFALSE);
      }
    }
    else
      request->send(500, TEXTPLAIN, BAD_PATH);
    return;

    // REQUEST: /update
  }
  else if (request->url() == UPDATE_PATH)
  {
    if (request->method() == HTTP_GET)
    {
      request->send(200, "text/html", "<form method='POST' action='/update'>Version mit v eingeben: <input type='text' name='version'><br><br><input type='submit' value='Update'></form>");
    }
    else if (request->method() == HTTP_POST)
    {
      if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str()))
        return request->requestAuthentication();
      if (request->hasParam("version", true))
      {
        //ESP.wdtDisable();
        // use getParam(xxx, true) for form-data parameters in POST request header
        String version = request->getParam("version", true)->value();
        Serial.println(version);
        if (version.indexOf("v") == 0)
        {
          gSystem->otaUpdate.get = version; // Versionswunsch speichern
          if (gSystem->otaUpdate.get == gSystem->otaUpdate.version)
            gSystem->otaUpdate.state = 1; // Version schon bekannt, direkt los
          else
            gSystem->otaUpdate.state = -1; // Version erst vom Server anfragen
          //ESP.wdtEnable(10);
        }
        else
          request->send(200, TEXTPLAIN, "Version unknown!");
      }
      else
      {
        gSystem->otaUpdate.get = gSystem->otaUpdate.version;
        gSystem->otaUpdate.state = 1; // Version bekannt, also direkt los
      }
      request->send(200, TEXTPLAIN, "Do Update...");
    }
    else
      request->send(500, TEXTPLAIN, BAD_PATH);
    return;

    // REQUEST: /upload
  }
  else if (request->url() == HISTORY)
  {
    if (request->method() == HTTP_GET)
    {
      File historyData = gSystem->sdCard->getHistoryData();
      if (historyData)
        request->send(historyData, "application/octet-stream");
      else
        request->send(500, TEXTPLAIN, BAD_PATH);
    }
    else
      request->send(500, TEXTPLAIN, BAD_PATH);
    return;

    // REQUEST: File from SPIFFS
  }
  else if (request->method() == HTTP_GET)
  {
    String path = request->url();
    if (path.endsWith("/"))
      path += DEFAULT_INDEX_FILE;
    //else if (path.endsWith(CHART_LIB_PATH)) path = CHART_LIB_PATH;
    if (request->url().equals("/"))
    {
      //sendFile(request,path); //
      request->send(SPIFFS, path);
      return;
    }
    bool auth = true;
    for (byte i = 0; i < sizeof(public_list) / sizeof(const char *); i++)
    {
      if (path.equals(public_list[i]))
      {
        auth = false;
        break;
      }
    }
    if (auth && !request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str()))
      return request->requestAuthentication();
    //sendFile(request,path); //
    request->send(SPIFFS, path);
  }
}

bool NanoWebHandler::canHandle(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_GET)
  {
    if (request->url() == DATA_PATH || request->url() == SETTING_PATH || request->url() == NETWORK_LIST || request->url() == NETWORK_SCAN || request->url() == NETWORK_STOP || request->url() == NETWORK_CLEAR || request->url() == CONFIG_RESET || request->url() == RECOVERY ||request->url() == UPDATE_PATH || request->url() == UPDATE_CHECK || request->url() == ADMIN || request->url() == UPLOAD || request->url() == HISTORY
        //|| request->url() == LOGGING_PATH
    )
    {
      return true;
    }
    else
    {
      // get file
      String path = request->url();
      if (path.endsWith("/"))
        path += DEFAULT_INDEX_FILE;

      //if(fileExists(path)) return true;
      if (SPIFFS.exists(path))
        return true;
    }
  }
  else if (request->method() == HTTP_DELETE && request->url() == DELETE_PATH)
  {
    return true;
  }
  else if (request->method() == HTTP_POST)
  {
    if (request->url() == FPUTS_PATH || request->url() == FLIST_PATH || request->url() == DATA_PATH || request->url() == SETTING_PATH || request->url() == NETWORK_LIST || request->url() == NETWORK_SCAN || request->url() == NETWORK_STOP || request->url() == NETWORK_CLEAR || request->url() == CONFIG_RESET || request->url() == UPDATE_PATH || request->url() == UPDATE_CHECK || request->url() == UPDATE_STATUS || request->url() == DC_STATUS || request->url() == ADMIN || request->url() == UPLOAD
        //|| request->url() == LOGGING_PATH
    )
      return true;
  }
  return false;
}

int BodyWebHandler::checkStringLength(String tex)
{
  int index = tex.length();
  while (tex.lastIndexOf(195) != -1)
  {
    tex.remove(tex.lastIndexOf(195), 1);
    index--;
  }
  return index;
}

String BodyWebHandler::checkString(String tex)
{
  tex.replace("&amp;", "&"); // &
  tex.replace("&lt;", "<");  // <
  tex.replace("&gt;", ">");  // >
  return tex;
}

bool BodyWebHandler::setSystem(AsyncWebServerRequest *request, uint8_t *datas)
{

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &_system = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!_system.success())
    return 0;

  String unit, _name;

  if (_system.containsKey("language"))
    gSystem->setLanguage(_system["language"].asString());
  if (_system.containsKey("unit"))
    unit = _system["unit"].asString();
  if (_system.containsKey("autoupd"))
    gSystem->otaUpdate.setAutoUpdate((boolean)_system["autoupd"]);
  if (_system.containsKey("prerelease"))
  {
    gSystem->otaUpdate.setPrerelease(_system["prerelease"]);
    gSystem->otaUpdate.saveConfig();
  }

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

  if (gSystem->temperatures.setUnit((TemperatureUnit)unit.charAt(0)))
  {
    gSystem->temperatures.saveConfig();
  }

  return 1;
}

bool BodyWebHandler::setChannels(AsyncWebServerRequest *request, uint8_t *datas)
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

bool BodyWebHandler::setNetwork(AsyncWebServerRequest *request, uint8_t *datas)
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

bool BodyWebHandler::addNetwork(AsyncWebServerRequest *request, uint8_t *datas)
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

bool BodyWebHandler::setIoT(AsyncWebServerRequest *request, uint8_t *datas)
{

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &_chart = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!_chart.success())
    return 0;

  MqttConfig mqttConfig = gSystem->mqtt.getConfig();
  CloudConfig cloudConfig = gSystem->cloud.getConfig();
  bool refresh = cloudConfig.enabled;

  if (_chart.containsKey("PMQhost"))
    mqttConfig.host = _chart["PMQhost"].asString();
  if (_chart.containsKey("PMQport"))
    mqttConfig.port = _chart["PMQport"];
  if (_chart.containsKey("PMQuser"))
    mqttConfig.user = _chart["PMQuser"].asString();
  if (_chart.containsKey("PMQpass"))
    mqttConfig.password = _chart["PMQpass"].asString();
  if (_chart.containsKey("PMQqos"))
    mqttConfig.QoS = _chart["PMQqos"];
  if (_chart.containsKey("PMQon"))
    mqttConfig.enabled = _chart["PMQon"];
  if (_chart.containsKey("PMQint"))
    mqttConfig.interval = _chart["PMQint"];

  gSystem->mqtt.setConfig(mqttConfig);

  if (_chart.containsKey("CLon"))
    cloudConfig.enabled = _chart["CLon"];
  if (_chart.containsKey("CLtoken"))
    cloudConfig.token = _chart["CLtoken"].asString();
  if (_chart.containsKey("CLint"))
    cloudConfig.interval = _chart["CLint"];

  gSystem->cloud.setConfig(cloudConfig);

  return 1;
}

bool BodyWebHandler::setPush(AsyncWebServerRequest *request, uint8_t *datas)
{

  printRequest(datas);

  DynamicJsonBuffer jsonBuffer;
  JsonObject &_push = jsonBuffer.parseObject((const char *)datas); //https://github.com/esp8266/Arduino/issues/1321
  if (!_push.success())
    return 0;

  if (_push.containsKey("on"))
    gSystem->notification.pushService.on = (byte)_push["on"];
  if (_push.containsKey("token"))
    gSystem->notification.pushService.token = _push["token"].asString();
  if (_push.containsKey("id"))
    gSystem->notification.pushService.id = _push["id"].asString();
  if (_push.containsKey("repeat"))
    gSystem->notification.pushService.repeat = _push["repeat"];
  if (_push.containsKey("service"))
    gSystem->notification.pushService.service = _push["service"];

  if (gSystem->notification.pushService.on == 2)
    gSystem->notification.notificationData.type = 1; // Verbindungstest
  else
  {
    gSystem->notification.saveConfig();
  }
  return 1;
}

bool BodyWebHandler::setPitmaster(AsyncWebServerRequest *request, uint8_t *datas)
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
        pm->disableActuators();
        //Serial.println("PID-Wechsel");
      }
      pm->assignProfile(gSystem->profile[temppid]);
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

bool BodyWebHandler::setPID(AsyncWebServerRequest *request, uint8_t *datas)
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
    if (id >= (sizeof(gSystem->profile)/sizeof(PitmasterProfile*)))
      break;

    PitmasterProfile *profile = gSystem->profile[id];

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
      if (val >= SERVOPULSMIN && val <= SERVOPULSMAX && profile->actuator == SERVO)
      {
        profile->dcmin = getDC(val * 10) / 10.0;
      }
      else
        profile->dcmin = constrain(val * 10, 0, 1000) / 10.0; // 1. Nachkommastelle
    }
    if (_pid.containsKey("DCmmax"))
    {
      val = _pid["DCmmax"];
      if (val >= SERVOPULSMIN && val <= SERVOPULSMAX && profile->actuator == SERVO)
      {
        profile->dcmax = getDC(val * 10) / 10.0;
      }
      else
        profile->dcmax = constrain(val * 10, 0, 1000) / 10.0; // 1. Nachkommastelle
    }
    if (_pid.containsKey("opl"))
      profile->opl = _pid["opl"];
    if (_pid.containsKey("tune"))
      profile->autotune = _pid["tune"];
    if (_pid.containsKey("jp"))
      profile->jumppw = constrain(_pid["jp"], 10, 100);

    ii++;
  }

  gSystem->pitmasters.saveConfig();

  return 1;
}

bool BodyWebHandler::setServerAPI(AsyncWebServerRequest *request, uint8_t *datas)
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
    JsonObject &_update = json["update"];
    if (_update.containsKey("available"))
      available = _update["available"];

    if (available && (gSystem->otaUpdate.getAutoUpdate() || gSystem->otaUpdate.get != "false"))
    {
      // bei gSystem->otaUpdate.get wurde eine bestimmte Version angefragt
      String version;
      if (_update.containsKey("version"))
      {
        version = _update["version"].asString();
        if (gSystem->otaUpdate.get == version)
        {
          if (gSystem->otaUpdate.state < 1)
            gSystem->otaUpdate.state = 1; // Anfrage erfolgreich, Update starten
          else if (gSystem->otaUpdate.state == 2)
            gSystem->otaUpdate.state = 3; // Anfrage während des Updateprozesses
        }
        else
        { // keine konrekte Anfrage
          gSystem->otaUpdate.version = version;
          gSystem->otaUpdate.get = "false"; // nicht die richtige Version übermittelt
        }
      }
      if (_update.containsKey("NX3224K028"))
      { // Firmware-Link
        JsonObject &_fw = _update["NX3224K028"];
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
        gSystem->otaUpdate.get = gSystem->otaUpdate.version;
        gSystem->otaUpdate.state = 1; // Update erzwingen
      }
      gSystem->otaUpdate.start();
    }
    else
    {
      gSystem->otaUpdate.version = "false"; // kein Server-Update
      if (gSystem->otaUpdate.get != "false")
        gSystem->otaUpdate.get = "false"; // bestimmte Version nicht bekannt
    }

    if (gSystem->otaUpdate.state == 3)
    {
    } // nicht speichern falls Absturz -> wiederholen
    else
    {
      gSystem->otaUpdate.saveConfig();
      return 0; // für Update
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

bool BodyWebHandler::setDCTest(AsyncWebServerRequest *request, uint8_t *datas)
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

BodyWebHandler::BodyWebHandler(void)
{
}

void BodyWebHandler::handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{

  if (request->url() == SET_NETWORK)
  {
    if (!setNetwork(request, data))
      request->send(200, TEXTPLAIN, TEXTFALSE);
    request->send(200, TEXTPLAIN, TEXTTRUE);
    return;
  }
  else if (request->url() == SET_CHANNELS)
  {
    if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str()))
      return request->requestAuthentication();
    if (!setChannels(request, data))
      request->send(200, TEXTPLAIN, TEXTFALSE);
    request->send(200, TEXTPLAIN, TEXTTRUE);
    return;
  }
  else if (request->url() == SET_SYSTEM)
  {
    if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str()))
      return request->requestAuthentication();
    if (!setSystem(request, data))
      request->send(200, TEXTPLAIN, TEXTFALSE);
    request->send(200, TEXTPLAIN, TEXTTRUE);
    return;
  }
  else if (request->url() == SET_PITMASTER)
  {
    if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str()))
      return request->requestAuthentication();
    if (!setPitmaster(request, data))
      request->send(200, TEXTPLAIN, TEXTFALSE);
    request->send(200, TEXTPLAIN, TEXTTRUE);
    return;
  }
  else if (request->url() == SET_PID)
  {
    if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str()))
      return request->requestAuthentication();
    if (!setPID(request, data))
      request->send(200, TEXTPLAIN, TEXTFALSE);
    request->send(200, TEXTPLAIN, TEXTTRUE);
    return;
  }
  else if (request->url() == SET_IOT)
  {
    if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str()))
      return request->requestAuthentication();
    if (!setIoT(request, data))
      request->send(200, TEXTPLAIN, TEXTFALSE);
    request->send(200, TEXTPLAIN, TEXTTRUE);
    return;
  }
  else if (request->url() == SET_PUSH)
  {
    if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str()))
      return request->requestAuthentication();
    if (!setPush(request, data))
      request->send(200, TEXTPLAIN, TEXTFALSE);
    request->send(200, TEXTPLAIN, TEXTTRUE);
    return;
  }
  else if (request->url() == SET_API)
  {
    if (!request->authenticate(WServer::getUsername().c_str(), WServer::getPassword().c_str()))
      return request->requestAuthentication();
    if (!setServerAPI(request, data))
      request->send(200, TEXTPLAIN, TEXTFALSE);
    request->send(200, TEXTPLAIN, TEXTTRUE);
  }
  else if (request->url() == SET_DC)
  {
    if (!setDCTest(request, data))
      request->send(200, TEXTPLAIN, TEXTFALSE);
    request->send(200, TEXTPLAIN, TEXTTRUE);
  }
}

bool BodyWebHandler::canHandle(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_GET)
    return false;
  if (request->url() == SET_NETWORK || request->url() == SET_CHANNELS || request->url() == SET_SYSTEM || request->url() == SET_PITMASTER || request->url() == SET_PID || request->url() == SET_IOT || request->url() == SET_API || request->url() == SET_DC || request->url() == SET_PUSH || request->url() == SET_GOD)
    return true;
  return false;
}
