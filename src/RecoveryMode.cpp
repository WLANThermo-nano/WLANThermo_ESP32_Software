/*************************************************** 
    Copyright (C) 2020  Martin Koerner

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
#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <Preferences.h>
#include <rom/rtc.h>
#include "ESPNexUpload.h"
#include "RecoveryMode.h"
#include "DbgPrint.h"
#include "Settings.h"
#include "system/SystemBase.h"
#include "webui/recoverymode.html.gz.h"
#include "webui/restart.html.gz.h"

#define RECOVERY_RESET_THRESHOLD 10u
#define RECOVERY_PIN 14u
#define RECOVERY_PIN_TIME 3000u // 3s
#define RECOVERY_AP_NAME "WLANThermo-RecoveryMode"
#define RECOVERY_AP_PASSWORD "12345678"
#define TEXTPLAIN "text/plain"
#define TEXTTRUE "true"

UploadFileType RecoveryMode::uploadFileType = UploadFileType::None;
void *RecoveryMode::nexUpload = NULL;
uint32_t RecoveryMode::nexBaudRate = 115200u;
size_t RecoveryMode::uploadFileSize = 0u;
String RecoveryMode::settingsKey = "";
String RecoveryMode::settingsValue = "";
RTC_DATA_ATTR boolean RecoveryMode::fromApp = false;
RTC_DATA_ATTR char RecoveryMode::wifiName[33];
RTC_DATA_ATTR char RecoveryMode::wifiPassword[64];
RTC_NOINIT_ATTR uint8_t RecoveryMode::resetCounter = 0u;

RecoveryMode::RecoveryMode(void)
{

}

void RecoveryMode::runFromApp(const char *paramWifiName, const char *paramWifiPassword)
{
  fromApp = true;
  strcpy(wifiName, paramWifiName);
  strcpy(wifiPassword, paramWifiPassword);

  delay(500);
  WiFi.disconnect();
  delay(500);

  esp_sleep_enable_timer_wakeup(10);
  esp_deep_sleep_start();
}

void RecoveryMode::run()
{
#if RM_DEBUG == SERIAL_DEBUG
  // Initialize Serial
  Serial.begin(115200);
  Serial.setDebugOutput(true);
#endif

  // Handle reset counter
  boolean initResetCounter = (POWERON_RESET == rtc_get_reset_reason(0)) || (DEEPSLEEP_RESET == rtc_get_reset_reason(0));
  resetCounter = (initResetCounter) ? 0u : resetCounter + 1u;
  RMPRINTF("Reset counter: %d\n", resetCounter);

  RMPRINTLN("Check for Recovery Mode");

  uint32_t startTime = millis();
  pinMode(RECOVERY_PIN, INPUT_PULLUP);

  while(((millis() - startTime) < RECOVERY_PIN_TIME) && (!fromApp) && (resetCounter < RECOVERY_RESET_THRESHOLD))
  {
    if(digitalRead(RECOVERY_PIN) == 1u)
    {
      // No recovery mode
      return;
    }
  }

  // Welcome to recovery mode
  resetCounter = 0u;
  RMPRINTLN("Recovery Mode enabled");

  WiFi.persistent(false);
  WiFi.disconnect(true);

  if(fromApp)
  {
    // increase nextion baud rate
    nexBaudRate = 460800u;

    // Start STA
    WiFi.begin(wifiName, wifiPassword);
    WiFi.mode(WIFI_STA);
    RMPRINTF("Recovery Mode starting Wifi STA. SSID: %s, PW: %s\n", wifiName, wifiPassword);

    while(WiFi.isConnected() == false)
    {
      RMPRINTLN("Wifi not connected");
      delay(1000);
    }

    RMPRINTF("IP address: %s\n", WiFi.localIP().toString().c_str());
  }
  else
  {
    // Start AP
    IPAddress local_IP(192, 168, 66, 1), gateway(192, 168, 66, 1), subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAP(RECOVERY_AP_NAME, RECOVERY_AP_PASSWORD);
    WiFi.mode(WIFI_AP);
    RMPRINTLN("Recovery Mode starting Wifi AP");
  }

  // Start web server
  AsyncWebServer *webServer = new AsyncWebServer(80);

  webServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", recoverymode_html_gz, sizeof(recoverymode_html_gz));
    response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  webServer->on("/recovery", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", recoverymode_html_gz, sizeof(recoverymode_html_gz));
    response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  webServer->on("/restart", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", restart_html_gz, sizeof(restart_html_gz));
    response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
    WiFi.disconnect();
    delay(1000);
    gSystem->restart();
  });

  webServer->on("/reset", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    Settings::clear();
    request->send(200, TEXTPLAIN, TEXTTRUE);
  });

  webServer->on("/ping", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    RMPRINTLN("GET /ping");
    request->send(200, TEXTPLAIN, WiFi.localIP().toString().c_str());
  });

  webServer->on("/export", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    RMPRINTLN("GET /export");
    String exportSettings = Settings::exportFile();
    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/text", (uint8_t*)exportSettings.c_str(), exportSettings.length());
    response->addHeader("Content-Disposition", "attachment; filename=settings.txt");
    response->addHeader("Connection", "close");
    request->send(response);
  });

  webServer->on("/import", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    RMPRINTLN("POST /import");
    if((request->contentLength() == 0u) && (request->hasHeader("xKey") == true))
    {
      Settings::remove(request->header("xKey"));
    }
    request->send(200, TEXTPLAIN, TEXTTRUE);
  }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
  {
    std::unique_ptr<char[]> s(new char[len + 1]);
    static size_t receivedBytes = 0u;

    if(!index) settingsKey = request->header("xKey"); settingsValue = ""; receivedBytes = 0u;
    memset(s.get(), 0, len + 1u);
    memcpy(s.get(), data, len);
    settingsValue += s.get();
    receivedBytes += len;
    if(receivedBytes == total) Settings::write(settingsKey, settingsValue);
  });

  webServer->on("/uploadfile", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    if (request->hasArg("usize"))
    {
      String usize = request->arg("usize");
      uploadFileSize = usize.toInt();
    }
    else if(nexUpload)
    {
      delete(nexUpload);
      nexUpload = NULL;
    }

    request->send(200, TEXTPLAIN, TEXTTRUE);

  },  [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
  {
    if(!index)
      uploadFileType = getFileType(filename);

    switch(uploadFileType)
    {
      case UploadFileType::Firmware:
        if(!index) Update.begin(uploadFileSize);
        Update.write(data, len);
        if(final) Update.end(true);
        break;
      case UploadFileType::SPIFFS:
        if(!index) Update.begin(uploadFileSize, U_SPIFFS);
        Update.write(data, len);
        if(final) Update.end(true);
        break;
#if defined HW_MINI_V1 || defined HW_MINI_V2 || defined HW_MINI_V3
      case UploadFileType::Nextion:
        if(!index) { nexUpload = new ESPNexUpload(nexBaudRate); ((ESPNexUpload*)nexUpload)->prepareUpload(uploadFileSize); }
        ((ESPNexUpload*)nexUpload)->upload(data, len);
        if(final) { ((ESPNexUpload*)nexUpload)->end(); delete(nexUpload); nexUpload = NULL; }
        break;
#endif
    }
  });

  // 404 not found
  webServer->onNotFound([](AsyncWebServerRequest *request) {
    request->send(404);
  });

  webServer->begin();

  while(true)
  {
    delay(100);
  }
}

UploadFileType RecoveryMode::getFileType(String fileName)
{
  UploadFileType retFileType = UploadFileType::None;

  if((fileName.indexOf("firmware") >= 0) && (fileName.indexOf(".bin") >= 0))
  {
    retFileType = UploadFileType::Firmware;
    RMPRINTLN("FILETYPE: Firmware");
  }
  else if((fileName.indexOf("spiffs") >= 0) && (fileName.indexOf(".bin") >= 0))
  {
    retFileType = UploadFileType::SPIFFS;
    RMPRINTLN("FILETYPE: SPIFFS");
  }
  else if(fileName.indexOf(".tft") >= 0)
  {
    retFileType = UploadFileType::Nextion;
    RMPRINTLN("FILETYPE: Nextion");
  }
  else
  {
    RMPRINTLN("FILETYPE: None");
  }

  return retFileType;
}