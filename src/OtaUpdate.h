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
  InProgress,
  Failed,
  Finished
};

class OtaUpdate
{
public:
  OtaUpdate();
  void saveConfig();
  void loadConfig();
  void setFirmwareUrl(const char *url);
  void setDisplayUrl(const char *url);
  void setAutoUpdate(boolean enable);
  boolean getAutoUpdate();
  void start();
  boolean getPrerelease();
  boolean setPrerelease(boolean prerelease);
  byte count;               // UPDATE SPIFFS REPEAT
  int state;                // UPDATE STATE: -1 = check, 0 = no, 1 = start spiffs, 2 = check after restart, 3 = firmware, 4 = finish
  String get;               // UPDATE MY NEW VERSION (über Eingabe)
  String version = "false"; // UPDATE SERVER NEW VERSION

private:
  static void task(void *parameter);
  void doFirmwareUpdate();
  void doDisplayUpdate();
  void downloadFileToSPIFFS(const char *url, const char *fileName);
  bool prerelease;
  bool autoUpdate;
  String firmwareUrl;
  String displayUrl;
  OtaUpdateState otaUpdateState;
};
