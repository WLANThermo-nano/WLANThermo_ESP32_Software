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

#include "Settings.h"
#include "nvs.h"
#include "Preferences.h"

#define STRINGIFY(s) #s

typedef struct
{
  const char *keyName;
  boolean exportEnabled;
  boolean debugPrint;
} NvsKeyConfig_t;

static const NvsKeyConfig_t NvsKeyConfig[] = {
    {STRINGIFY(kWifi), false, true},
    {STRINGIFY(kMqtt), false, true},
    {STRINGIFY(kCloud), false, true},
    {STRINGIFY(kSystem), true, true},
    {STRINGIFY(kChannels), true, false},
    {STRINGIFY(kPitmasters), true, true},
    {STRINGIFY(kPush), false, true},
    {STRINGIFY(kDisplay), true, true},
    {STRINGIFY(kBattery), true, true},
    {STRINGIFY(kOtaUpdate), true, true},
    {STRINGIFY(kServer), true, true},
    {STRINGIFY(kBluetooth), true, true},
    {STRINGIFY(kConnect), true, true},
    {STRINGIFY(kPbGuard), true, true}};

const char *Settings::nvsNamespace = "wlanthermo";
const uint16_t Settings::jsonBufferSize = 3072u;
std::vector<SettingsOnChangeCallback> Settings::registeredCallbacks;

Settings::Settings()
{
}

void Settings::write(SettingsNvsKeys key, JsonObject &json)
{
  Preferences prefs;
  String jsonString;
  json.printTo(jsonString);
  prefs.begin(nvsNamespace, false);
  prefs.putString(NvsKeyConfig[key].keyName, jsonString);
  prefs.end();

  if (NvsKeyConfig[key].debugPrint)
  {
    Serial.printf("Settings::write: %s - %s\n", NvsKeyConfig[key].keyName, jsonString.c_str());
  }

  for (std::vector<SettingsOnChangeCallback>::iterator it = registeredCallbacks.begin(); it != registeredCallbacks.end(); ++it)
    (*it)(key);
}

JsonObject &Settings::read(SettingsNvsKeys key, DynamicJsonBuffer *jsonBuffer)
{
  Preferences prefs;
  String jsonString;
  prefs.begin(nvsNamespace, true);
  jsonString = prefs.getString(NvsKeyConfig[key].keyName, "");
  JsonObject &json = jsonBuffer->parseObject(jsonString);
  prefs.end();

  if (NvsKeyConfig[key].debugPrint)
  {
    Serial.printf("Settings::read: %s (%d bytes) - %s\n", NvsKeyConfig[key].keyName, jsonString.length(), jsonString.c_str());
  }

  return json;
}

String Settings::exportFile()
{
  String exportString = "";

  for (uint8_t keyIndex = 0u; keyIndex < (sizeof(NvsKeyConfig) / sizeof(NvsKeyConfig_t)); keyIndex++)
  {
    if (NvsKeyConfig[keyIndex].exportEnabled)
    {
      Preferences prefs;
      String jsonString;
      prefs.begin(nvsNamespace, true);
      jsonString = prefs.getString(NvsKeyConfig[keyIndex].keyName, "");
      prefs.end();
      exportString += NvsKeyConfig[keyIndex].keyName;
      exportString += ":";
      exportString += jsonString;
      exportString += "\r\n";
    }
  }
  return exportString;
}

void Settings::write(String key, String value)
{
  for (uint8_t keyIndex = 0u; keyIndex < (sizeof(NvsKeyConfig) / sizeof(NvsKeyConfig_t)); keyIndex++)
  {
    if (String(NvsKeyConfig[keyIndex].keyName) == key)
    {
      Preferences prefs;
      prefs.begin(nvsNamespace, false);
      prefs.putString(key.c_str(), value.c_str());

      if (NvsKeyConfig[keyIndex].debugPrint)
      {
        Serial.printf("Settings::write: %s - %s\n", key.c_str(), value.c_str());
      }

      prefs.end();
      break;
    }
  }
}

void Settings::remove(SettingsNvsKeys key)
{
  Preferences prefs;
  prefs.begin(nvsNamespace);
  prefs.remove(NvsKeyConfig[key].keyName);
  Serial.printf("Settings::remove: %s\n", NvsKeyConfig[key].keyName);
  prefs.end();
}

void Settings::remove(String key)
{
  Preferences prefs;
  prefs.begin(nvsNamespace);
  prefs.remove(key.c_str());
  Serial.printf("Settings::remove: %s\n", key.c_str());
  prefs.end();
}

void Settings::clear()
{
  Preferences prefs;
  prefs.begin(nvsNamespace);
  prefs.clear();
  Serial.printf("Settings::clear\n");
  prefs.end();
}

void Settings::onWrite(SettingsOnChangeCallback cb)
{
  registeredCallbacks.push_back(cb);
}