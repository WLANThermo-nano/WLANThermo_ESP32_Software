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
#include "RecoveryMode.h"
#include "Settings.h"
#include "DbgPrint.h"
#include "TaskConfig.h"

OtaUpdate::OtaUpdate()
{
  prerelease = false;
  autoUpdate = true;
  firmwareUrl = "";
  displayUrl = "";
  otaUpdateState = OtaUpdateState::Idle;
  requestedVersion = "false";
  version = "false";
  requestedFile = "";
  forceUpdate = false;
}

void OtaUpdate::startUpdate()
{
  otaUpdateState = OtaUpdateState::UpdateInProgress;
  xTaskCreatePinnedToCore(OtaUpdate::task, "OtaUpdate::task", 10000, this, TASK_PRIORITY_OTA_UPDATE, NULL, 1);
}

void OtaUpdate::task(void *parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  OtaUpdate *otaUpdate = (OtaUpdate *)parameter;
  boolean success = false;

  Serial.println("OTA update has been started.");

  success = otaUpdate->doFirmwareUpdate();

  if (success)
    success = otaUpdate->doDisplayUpdate();

  if (success)
  {
    Serial.println("OTA update finished.");
    RecoveryMode::zeroResetCounter();
    ESP.restart();
  }

  if (!success)
  {
    Serial.println("OTA update failed.");
    otaUpdate->otaUpdateState = OtaUpdateState::UpdateFailed;
  }

  vTaskDelete(NULL);
}

void OtaUpdate::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();
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
    if (json.containsKey("autoupd"))
      autoUpdate = json["autoupd"];
    if (json.containsKey("prerelease"))
      prerelease = json["prerelease"];
  }
}

boolean OtaUpdate::checkForUpdate(String version)
{
  boolean doUpdate = false;

  if ((this->version != "false") && (this->version == this->requestedVersion))
  {
    doUpdate = true;
    startUpdate();
  }

  return doUpdate;
}

void OtaUpdate::requestVersion(String requestedVersion)
{
  Serial.printf("OtaUpdate::requestVersion: %s\n", requestedVersion.c_str());
  this->requestedVersion = requestedVersion;
}

void OtaUpdate::requestFile(String file)
{
  Serial.printf("OtaUpdate::requestFile: %s\n", file.c_str());
  this->requestedFile = file;
}

void OtaUpdate::resetUpdateInfo()
{
  firmwareUrl = "";
  displayUrl = "";
  requestedVersion = "false";
  version = "false";
  requestedFile = "";
  forceUpdate = false;
}

void OtaUpdate::update()
{
  switch (otaUpdateState)
  {
  case OtaUpdateState::Idle:
    otaUpdateState = (this->autoUpdate) ? OtaUpdateState::GetUpdateInfo : OtaUpdateState::Idle;
    break;
  case OtaUpdateState::GetUpdateInfo:
    Cloud::sendAPI(APIUPDATE, APILINK, NOPARA);
    otaUpdateState = OtaUpdateState::NoUpdateInfo;
    break;
  case OtaUpdateState::NoUpdateInfo:
    otaUpdateState = (version != "false") ? OtaUpdateState::UpdateAvailable : OtaUpdateState::NoUpdateInfo;
    break;
  case OtaUpdateState::UpdateAvailable:
    break;
  case OtaUpdateState::UpdateInProgress:
    break;
  case OtaUpdateState::UpdateFinished:
    break;
  case OtaUpdateState::UpdateFailed:
  default:
    break;
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
  if ((false == this->autoUpdate) && (true == enable))
  {
    resetUpdateInfo();
    askUpdateInfo();
  }

  this->autoUpdate = enable;
}

boolean OtaUpdate::getAutoUpdate()
{
  return this->autoUpdate;
}

boolean OtaUpdate::downloadFileToSPIFFS(const char *url, const char *fileName)
{
  boolean success = false;

  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return false;
  }

  File nextionFile = SPIFFS.open(fileName, FILE_WRITE);

  if (!nextionFile)
  {
    Serial.println("Error opening file");
    return false;
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
      if (http.writeToStream(&nextionFile) > 0)
        success = true;
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

  return success;
}

boolean OtaUpdate::doFirmwareUpdate()
{
  boolean success = false;

  if (!this->firmwareUrl.length())
    return true;

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
    success = true;
    break;
  }

  return success;
}

boolean OtaUpdate::doDisplayUpdate()
{
  if (!this->displayUrl.length())
    return true;

  return this->downloadFileToSPIFFS(this->displayUrl.c_str(), "/nextion.tft.zlib");
}

boolean OtaUpdate::getPrerelease()
{
  return prerelease;
}

boolean OtaUpdate::setPrerelease(boolean prerelease)
{
  boolean checkForUpdate = (false == this->prerelease) && (true == prerelease) && (true == this->autoUpdate);
  this->prerelease = prerelease;

  if (checkForUpdate)
    resetUpdateInfo();
}

uint8_t OtaUpdate::getUpdateProgress()
{
  static uint8_t progress = 0u;
  uint8_t newProgress = (uint8_t)((100.0f / ((float)Update.size())) * (float)Update.progress());

  if (Update.isRunning() && (newProgress > progress))
  {
    progress = newProgress;
  }

  return progress;
};