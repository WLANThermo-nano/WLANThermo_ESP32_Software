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
#include "ESPNexUpload.h"
#include "RecoveryMode.h"
#include "webui/recoverymode.html.gz.h"

#define RECOVERY_PIN 14u
#define RECOVERY_PIN_TIME 3000u // 3s
#define RECOVERY_AP_NAME "WLANThermo-RecoveryMode"
#define RECOVERY_AP_PASSWORD "12345678"
#define TEXTPLAIN "text/plain"
#define TEXTTRUE "true"

UploadFileType RecoveryMode::uploadFileType = UploadFileType::None;
void *RecoveryMode::nexUpload = NULL;
size_t RecoveryMode::uploadFileSize = 0u;

RecoveryMode::RecoveryMode(void)
{

}

void RecoveryMode::run()
{
  uint32_t startTime = millis();
  pinMode(RECOVERY_PIN, INPUT_PULLUP);

  while((millis() - startTime) < RECOVERY_PIN_TIME)
  {
    if(digitalRead(RECOVERY_PIN) == 1u)
    {
      // No recovery mode
      return;
    }
  }

  // Welcome to recovery mode

  // Start AP
  IPAddress local_IP(192, 168, 66, 1), gateway(192, 168, 66, 1), subnet(255, 255, 255, 0);
  WiFi.persistent(false);
  WiFi.disconnect(true);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(RECOVERY_AP_NAME, RECOVERY_AP_PASSWORD);
  WiFi.mode(WIFI_AP);

  // Start web server
  AsyncWebServer *webServer = new AsyncWebServer(80);

  webServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", recoverymode_html_gz, sizeof(recoverymode_html_gz));
    response->addHeader("Content-Disposition", "inline; filename=\"index.html\"");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  webServer->on("/restart", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    request->send(200, TEXTPLAIN, TEXTTRUE);
    ESP.restart();
  });

  webServer->on("/resetconfig", HTTP_POST, [](AsyncWebServerRequest *request)
  {
    Preferences prefs;
    prefs.begin("wlanthermo", false);
    prefs.clear();
    prefs.end();

    request->send(200, TEXTPLAIN, TEXTTRUE);
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
#if defined HW_MINI_V2 || defined HW_MINI_V3
      case UploadFileType::Nextion:
        if(!index) { nexUpload = new ESPNexUpload(115200); ((ESPNexUpload*)nexUpload)->prepareUpload(uploadFileSize); }
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
  }
  else if((fileName.indexOf("spiffs") >= 0) && (fileName.indexOf(".bin") >= 0))
  {
    retFileType = UploadFileType::SPIFFS;
  }
  else if(fileName.indexOf(".tft") >= 0)
  {
    retFileType = UploadFileType::Nextion;
  }

  return retFileType;
}