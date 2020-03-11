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
#pragma once

#include "Arduino.h"

enum class OtaUpdateState
{
  Idle,
  GetUpdateInfo,
  NoUpdateInfo,
  UpdateAvailable,
  UpdateInProgress,
  UpdateFailed,
  UpdateFinished
};

class OtaUpdate
{
public:
  OtaUpdate();
  void saveConfig();
  void loadConfig();
  void update();
  void setFirmwareUrl(const char *url);
  void setDisplayUrl(const char *url);
  void setAutoUpdate(boolean enable);
  void setUpdateVersion(String version){this->version = version;};
  void requestVersion(String version);
  void resetUpdateInfo();
  String getRequestedVersion(){return this->requestedVersion;};
  String getVersion(){return this->version;};
  boolean checkForUpdate(String version);
  boolean getAutoUpdate();
  boolean isUpdateInProgress(){return (OtaUpdateState::UpdateInProgress == otaUpdateState);};
  void startUpdate();
  boolean getPrerelease();
  boolean setPrerelease(boolean prerelease);

private:
  static void task(void *parameter);
  boolean doFirmwareUpdate();
  boolean doDisplayUpdate();
  boolean downloadFileToSPIFFS(const char *url, const char *fileName);
  bool prerelease;
  bool autoUpdate;
  String firmwareUrl;
  String displayUrl;
  OtaUpdateState otaUpdateState;
  String requestedVersion;
  String version;
};
