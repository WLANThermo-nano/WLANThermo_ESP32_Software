/*************************************************** 
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
#include "ArduinoJson.h"

enum SettingsNvsKeys
{
  kWifi = 0,
  kMqtt,
  kCloud,
  kSystem,
  kChannels,
  kPitmasters,
  kPush,
  kDisplay,
  kBattery,
  kOtaUpdate,
  kServer,
  kBluetooth,
  kConnect,
  kPbGuard
};

typedef void (*SettingsOnChangeCallback)(SettingsNvsKeys);

class Settings
{
public:
  Settings();
  static void write(SettingsNvsKeys key, JsonObject &json);
  static void write(String key, String value);
  static JsonObject &read(SettingsNvsKeys key, DynamicJsonBuffer *jsonBuffer);
  static String exportFile();
  static void remove(SettingsNvsKeys key);
  static void remove(String key);
  static void clear();
  static void onWrite(SettingsOnChangeCallback cb);

  static const uint16_t jsonBufferSize;

private:
  static const char *nvsNamespace;
  static std::vector<SettingsOnChangeCallback> registeredCallbacks;
};
