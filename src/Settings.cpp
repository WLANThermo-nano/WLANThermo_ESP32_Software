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
#include <memory>

#define STRINGIFY(s) #s

typedef struct
{
  const char *keyName;
  boolean exportEnabled;
} NvsKeyConfig_t;

static const NvsKeyConfig_t NvsKeyConfig[] =
        {{STRINGIFY(kWifi),       false},
        {STRINGIFY(kMqtt),        false},
        {STRINGIFY(kCloud),       false},
        {STRINGIFY(kSystem),      true},
        {STRINGIFY(kChannels),    true},
        {STRINGIFY(kPitmasters),  true},
        {STRINGIFY(kPush),        false},
        {STRINGIFY(kDisplay),     true},
        {STRINGIFY(kBattery),     true},
        {STRINGIFY(kOtaUpdate),   true},
        {STRINGIFY(kServer),      true}};

class Prefs : public Preferences
{
public:
  Prefs(){};
  size_t getStringLength(const char *key)
  {
    size_t len = 0;

    if (!_started || !key)
      return 0;

    esp_err_t err = nvs_get_str(_handle, key, NULL, &len);

    if (err)
      return 0;

    return len;
  };
};

const char *Settings::nvsNamespace = "wlanthermo";
const uint16_t Settings::jsonBufferSize = 3072u;
std::vector<SettingsOnChangeCallback> Settings::registeredCallbacks;

Settings::Settings()
{
}

void Settings::write(SettingsNvsKeys key, JsonObject &json)
{
  Prefs prefs;
  size_t sSize = json.measureLength() + 1u;
  std::unique_ptr<char[]> s(new char[sSize]);
  json.printTo(s.get(), sSize);
  prefs.begin(nvsNamespace, false);
  prefs.putString(NvsKeyConfig[key].keyName, s.get());
  Serial.printf("Settings::write: %s - %s\n", NvsKeyConfig[key].keyName, s.get());
  prefs.end();

  for (std::vector<SettingsOnChangeCallback>::iterator it = registeredCallbacks.begin() ; it != registeredCallbacks.end(); ++it)
    (*it)(key);
}

JsonObject &Settings::read(SettingsNvsKeys key, DynamicJsonBuffer *jsonBuffer)
{
  Prefs prefs;
  prefs.begin(nvsNamespace, true);
  size_t sSize = prefs.getStringLength(NvsKeyConfig[key].keyName) + 1u;
  std::unique_ptr<char[]> s(new char[sSize]);
  memset(s.get(), 0, sSize);
  prefs.getString(NvsKeyConfig[key].keyName, s.get(), sSize);
  prefs.end();
  Serial.printf("Settings::read: %s (%d bytes) - %s\n", NvsKeyConfig[key].keyName, sSize, s.get());
  JsonObject &json = jsonBuffer->parseObject(s.get());
  return json;
}

String Settings::exportFile()
{
  String exportString = "";

  for(uint8_t keyIndex = 0u; keyIndex < (sizeof(NvsKeyConfig)/sizeof(NvsKeyConfig_t)); keyIndex++)
  {
    if(NvsKeyConfig[keyIndex].exportEnabled)
    {
      Prefs prefs;
      prefs.begin(nvsNamespace, true);
      size_t sSize = prefs.getStringLength(NvsKeyConfig[keyIndex].keyName) + 1u;
      std::unique_ptr<char[]> s(new char[sSize]);
      memset(s.get(), 0, sSize);
      prefs.getString(NvsKeyConfig[keyIndex].keyName, s.get(), sSize);
      exportString += NvsKeyConfig[keyIndex].keyName;
      exportString += ":";
      exportString += s.get();
      exportString += "\r\n";
      prefs.end();
    }
  }
  return exportString;
}

void Settings::write(String key, String value)
{
  for(uint8_t keyIndex = 0u; keyIndex < (sizeof(NvsKeyConfig)/sizeof(NvsKeyConfig_t)); keyIndex++)
  {
    if(String(NvsKeyConfig[keyIndex].keyName) == key)
    {
      Prefs prefs;
      prefs.begin(nvsNamespace, false);
      prefs.putString(key.c_str(), value.c_str());
      Serial.printf("Settings::write: %s - %s\n", key.c_str(), value.c_str());
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
  Serial.printf("Settings::remove: %s\n",key.c_str());
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