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

static char *NvsKeyStrings[] =
    {
        STRINGIFY(kWifi),
        STRINGIFY(kMqtt),
        STRINGIFY(kCloud),
        STRINGIFY(kSystem),
        STRINGIFY(kChannels),
        STRINGIFY(kPitmasters),
        STRINGIFY(kPush),
        STRINGIFY(kDisplay),
        STRINGIFY(kBattery),
        STRINGIFY(kOtaUpdate),
        STRINGIFY(kServer)
        };

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
  prefs.putString(NvsKeyStrings[key], s.get());
  Serial.printf("Settings::write: %s - %s\n", NvsKeyStrings[key], s.get());
  prefs.end();

  for (std::vector<SettingsOnChangeCallback>::iterator it = registeredCallbacks.begin() ; it != registeredCallbacks.end(); ++it)
    (*it)(key);
}

JsonObject &Settings::read(SettingsNvsKeys key, DynamicJsonBuffer *jsonBuffer)
{
  Prefs prefs;
  prefs.begin(nvsNamespace, true);
  size_t sSize = prefs.getStringLength(NvsKeyStrings[key]) + 1u;
  std::unique_ptr<char[]> s(new char[sSize]);
  memset(s.get(), 0, sSize);
  prefs.getString(NvsKeyStrings[key], s.get(), sSize);
  prefs.end();
  Serial.printf("Settings::read: %s (%d bytes) - %s\n", NvsKeyStrings[key], sSize, s.get());
  JsonObject &json = jsonBuffer->parseObject(s.get());
  return json;
}

void Settings::clear(SettingsNvsKeys key)
{
  Preferences prefs;
  prefs.begin(nvsNamespace, true);
  prefs.remove(NvsKeyStrings[key]);
  prefs.end();
}

void Settings::onWrite(SettingsOnChangeCallback cb)
{
  registeredCallbacks.push_back(cb);
}