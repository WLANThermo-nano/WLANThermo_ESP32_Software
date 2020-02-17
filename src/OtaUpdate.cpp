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

#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <SPIFFS.h>
#include "OtaUpdate.h"
#include "system/SystemBase.h"
#include "display/DisplayBase.h"
#include "Settings.h"
#include "DbgPrint.h"

OtaUpdate::OtaUpdate()
{
  state = 0;
  
  prerelease = false;
  autoUpdate = true;
  firmwareUrl = "";
  displayUrl = "";
  otaUpdateState = OtaUpdateState::Idle;


  get = "false"; // Änderungen am EE während Update
  version = "false";

  state = -1; // Kontakt zur API herstellen
}

void OtaUpdate::start()
{
  xTaskCreatePinnedToCore(OtaUpdate::task, "OtaUpdate::task", 10000, this, 100, NULL, 1);
}

void OtaUpdate::task(void *parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  OtaUpdate *otaUpdate = (OtaUpdate *)parameter;

  otaUpdate->doFirmwareUpdate();
  otaUpdate->doDisplayUpdate();

  vTaskDelete(NULL);
}

void OtaUpdate::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();
  json["update"] = state;
  json["getupd"] = get;
  json["autoupd"] = autoUpdate;
  json["prerelease"] = prerelease;
  Settings::write(kOtaUpdate, json);
}

void OtaUpdate::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kOtaUpdate, &jsonBuffer);

  if (json.success())
  {
    if (json.containsKey("update"))
      state = json["update"];
    if (json.containsKey("autoupd"))
      autoUpdate = json["autoupd"];
    if (json.containsKey("getupd"))
      get = json["getupd"].asString();
    if (json.containsKey("prerelease"))
      prerelease = json["prerelease"];
  }
}

void OtaUpdate::setFirmwareUrl(const char *url)
{
  firmwareUrl = url;
}

void OtaUpdate::setDisplayUrl(const char *url)
{
  displayUrl = url;
}

void OtaUpdate::setAutoUpdate(boolean enable)
{
  this->autoUpdate = enable;
}

boolean OtaUpdate::getAutoUpdate()
{
  return this->autoUpdate;
}

void OtaUpdate::downloadFileToSPIFFS(const char *url, const char *fileName)
{
   if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  File nextionFile = SPIFFS.open(fileName, FILE_WRITE);

  if (!nextionFile)
  {
    Serial.println("Error opening file");
    return;
  }

  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  http.addHeader("Accept-Encoding", "deflate");
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    Serial.printf("Content-Encoding: %s\n", http.header("Content-Encoding").c_str());

    if (httpCode == HTTP_CODE_OK)
    {
      http.writeToStream(&nextionFile);
    }
  }
  else
  {
    Serial.printf("Http GET failed: %s\n", http.errorToString(httpCode).c_str());
  }

  Serial.printf("File %s has been written to SPIFFS\n", fileName);

  http.end();
  nextionFile.close();
  SPIFFS.end();
}

void OtaUpdate::doFirmwareUpdate()
{
  if (!this->firmwareUrl.length())
    return;

  WiFiClient client;
  t_httpUpdate_return retVal;

  httpUpdate.rebootOnUpdate(false);
  retVal = httpUpdate.update(client, this->firmwareUrl);

  switch (retVal)
  {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");
    break;
  }
}

void OtaUpdate::doDisplayUpdate()
{
  if (!this->displayUrl.length())
    return;

  this->downloadFileToSPIFFS(this->displayUrl.c_str(), "/nextion.tft.zlib");
}

boolean OtaUpdate::getPrerelease()
{
  return prerelease;
}

boolean OtaUpdate::setPrerelease(boolean prerelease)
{
  this->prerelease = prerelease;
}