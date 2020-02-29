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
#pragma once
#include <Arduino.h>

enum class UploadFileType
{
  None,
  Firmware,
  SPIFFS,
  Nextion
};

class RecoveryMode
{
public:
  RecoveryMode(void);
  static void run();
  static void runFromApp(const char *paramWifiName, const char *paramWifiPassword);

private:
  static UploadFileType getFileType(String fileName);
  static UploadFileType uploadFileType;
  static size_t uploadFileSize;
  static void *nexUpload;
  static String settingsKey;
  static String settingsValue;
  static RTC_DATA_ATTR char wifiName[33];
  static RTC_DATA_ATTR char wifiPassword[64];
  static RTC_DATA_ATTR boolean fromApp;
};
