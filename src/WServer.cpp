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
    
    HISTORY: Please refer Github History
    
 ****************************************************/

// Beispiele:
// https://github.com/spacehuhn/wifi_ducky/blob/master/esp8266_wifi_duck/esp8266_wifi_duck.ino
// WebSocketClient: https://github.com/Links2004/arduinoWebSockets/issues/119

#include "WServer.h"
#include "WebHandler.h"
#include "Cloud.h"
#include "system/SystemBase.h"
#include "DbgPrint.h"
#include "Settings.h"
#include "ArduinoLog.h"
#include "RecoveryMode.h"
#include <SPIFFS.h>

// include html files
#include "webui/restart.html.gz.h"

#if defined(HW_MINI_V1) || defined(HW_MINI_V2) || defined(HW_MINI_V3) || defined(HW_CONNECT_V1)
#define WEB_SUBFOLDER "mini"
#elif HW_NANO_V3
#define WEB_SUBFOLDER "nano"
#elif HW_LINK_V1
#define WEB_SUBFOLDER "link"
#elif HW_BONE_V1
#define WEB_SUBFOLDER "bone"
#endif

extern const uint8_t index_html_start[] asm("_binary_webui_dist_"WEB_SUBFOLDER"_index_html_gz_start");
extern const size_t index_html_size asm("_binary_webui_dist_"WEB_SUBFOLDER"_index_html_gz_size");
extern const uint8_t favicon_ico_start[] asm("_binary_webui_dist_"WEB_SUBFOLDER"_favicon_ico_gz_start");
extern const size_t favicon_ico_size asm("_binary_webui_dist_"WEB_SUBFOLDER"_favicon_ico_gz_size");

const char *vueRouterPaths[] = {
    "/wlan", "/system", "/bluetooth", "/pitmaster", "/about", "/iot", "/notification", "/scan"};
static constexpr size_t kVueRouterPaths = sizeof(vueRouterPaths) / sizeof(vueRouterPaths[0]);

const char *WServer::username = "admin";
String WServer::password = "";
bool WServer::saveConfigPending = false;

WServer::WServer() : webServer(80)
{
}

void WServer::init()
{
  loadConfig();
  webServer.addHandler(&nanoWebHandler);

  webServer.on("/help", HTTP_GET, [](AsyncWebServerRequest *request) {
             request->redirect("https://github.com/WLANThermo-nano/WLANThermo_nano_Software/blob/master/README.md");
           })
      .setFilter(ON_STA_FILTER);

  webServer.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    char buf[512];
    int len = snprintf(buf, sizeof(buf),
        "spiffs: %u | %u\nheap: %u\nsn: %s\npn: %s\n",
        (unsigned)SPIFFS.usedBytes(), (unsigned)SPIFFS.totalBytes(),
        (unsigned)ESP.getFreeHeap(),
        gSystem->getSerialNumber().c_str(),
        gSystem->item.read(ItemNvsKeys::kItem).c_str());
    if (gSystem->battery != NULL && len < (int)sizeof(buf))
    {
      len += snprintf(buf + len, sizeof(buf) - len,
          "batlimit: %d | %d\nbat: %d | %d | %u\nbatstat: %d | %d\n",
          gSystem->battery->min, gSystem->battery->max,
          gSystem->battery->adcvoltage, gSystem->battery->voltage,
          (unsigned)gSystem->battery->simc,
          gSystem->battery->getPowerModeInt(), gSystem->battery->setreference);
    }
    if (len < (int)sizeof(buf))
    {
      snprintf(buf + len, sizeof(buf) - len,
          "ssid: %s\nwifimode: %d\nmac:%s\niS: %u",
          WiFi.SSID().c_str(), (int)WiFi.getMode(),
          gSystem->wlan.getMacAddress().c_str(),
          (unsigned)gSystem->getFlashSize());
    }
    request->send(200, "", buf);
  });

  webServer.on("/setbattmin", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (gSystem->battery)
    {
      if (gSystem->battery->min - 100 >= 2800)
      {
        gSystem->battery->min -= 100;
        gSystem->battery->saveConfigPending = true;
      }
    }
    request->send(200, TEXTPLAIN, "Done");
  });

  webServer.on("/settestmode", HTTP_GET, [](AsyncWebServerRequest *request) {
    CloudConfig cloudConfig = gSystem->cloud.getConfig();
    cloudConfig.cloudInterval = 3u;
    gSystem->cloud.setConfig(cloudConfig);
    request->send(200, TEXTPLAIN, "3 Sekunden");
  });

  webServer.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request) {
    for (uint8_t i = 0u; i < gSystem->pitmasters.count(); i++)
    {
      Pitmaster *pm = gSystem->pitmasters[i];

      if (pm != NULL)
        pm->setType(pm_off);
    }
    gSystem->pitmasters.saveConfigPending = true;
    request->send(200, TEXTPLAIN, "Stop pitmaster");
  });

  webServer.on("/clientlog", HTTP_GET, [](AsyncWebServerRequest *request) {
    Cloud::clientlog = true;
    gSystem->otaUpdate.resetUpdateInfo();
    gSystem->otaUpdate.askUpdateInfo();
    request->send(200, TEXTPLAIN, "aktiviert");
  });

  webServer.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
             AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", restart_html_gz, sizeof(restart_html_gz));
             response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
             response->addHeader("Content-Encoding", "gzip");
             request->send(response);
             gSystem->restartDeferred();
           })
      .setFilter(ON_STA_FILTER);

  webServer.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, TEXTPLAIN, WiFi.localIP().toString().c_str());
  });

  webServer.on("/newtoken", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, TEXTPLAIN, gSystem->cloud.newToken());
    gSystem->cloud.requestSaveConfig();
  });

  webServer.on("/rr", HTTP_GET, [](AsyncWebServerRequest *request) {
    String response = "\nCPU0: " + gSystem->getResetReason(0);
    response += "\nCPU1: " + gSystem->getResetReason(1);
    response += "\nResetCounter: " + String(RecoveryMode::getResetCounter());
    request->send(200, TEXTPLAIN, response);
  });

  // to avoid multiple requests to ESP
  webServer.on("/", [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_start, (size_t)&index_html_size);
    response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  // favicon.ico
  webServer.on("/favicon.ico", [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", favicon_ico_start, (size_t)&favicon_ico_size);
    response->addHeader("Content-Disposition", "inline; filename=\"favicon.ico\"");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  // 404 NOT found: called when the url is not defined here
  webServer.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS)
    {
      request->send(200);
    }
    else
    {
      boolean isVueRouterUrl = false;
      for(uint8_t index = 0u; index < kVueRouterPaths; index++)
      {
        if(request->url() == vueRouterPaths[index])
        {
          isVueRouterUrl = true;
          break;
        }
      }

      if(true == isVueRouterUrl)
      {
        // send index.html
        AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", index_html_start, (size_t)&index_html_size);
        response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
      }
      else
      {
        request->send(404);
      }
    }
  });

  /* Add default headers */
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS, DELETE, PUT");
  DefaultHeaders::Instance().addHeader("Access-Control-Max-Age", "1000");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "x-requested-with, Content-Type, origin, authorization, accept, client-security-token, scan");
    
  webServer.begin();
  Log.notice("HTTP server started" CR);
}

void WServer::update()
{
  if (saveConfigPending)
  {
    JsonDocument doc;
    JsonObject json = doc.to<JsonObject>();
    json["password"] = password;
    Settings::write(kServer, json);
    saveConfigPending = false;
  }
}

void WServer::saveConfig()
{
  saveConfigPending = true;
}

void WServer::loadConfig()
{
  JsonDocument doc;
  JsonObject json = Settings::read(kServer, doc);

  if (!json.isNull())
  {

    if (json.containsKey("password"))
    {
      const char *pw = json["password"].as<const char *>();
      if (pw != nullptr)
        this->password = pw;
      else
        Log.error("WServer::loadConfig: password key has unexpected type" CR);
    }
  }
}

String WServer::getUsername()
{
  return username;
}

String WServer::getPassword()
{
  return password;
}

void WServer::setPassword(String newPassword)
{
  password = newPassword;
}

WServer gWebServer;
