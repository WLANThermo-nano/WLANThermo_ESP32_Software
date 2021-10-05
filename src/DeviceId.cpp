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

#include "DeviceId.h"
#include "nvs.h"
#include "Preferences.h"
#include "ESPRandom.h"
#include <memory>

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

const char *DeviceId::nvsDeviceIdNamespace = "DeviceId";
const char *DeviceId::nvsDeviceKey = "kDeviceId";

DeviceId::DeviceId()
{
}

void DeviceId::init()
{
  if(DeviceId::get() == "")
  {
    uint8_t uuid[16];
    Prefs prefs;

    ESPRandom::uuid(uuid);
    prefs.begin(nvsDeviceIdNamespace, false);
    prefs.putString(nvsDeviceKey, ESPRandom::uuidToString(uuid).c_str());
    prefs.end();
  }

}

String DeviceId::get()
{
  Prefs prefs;
  prefs.begin(nvsDeviceIdNamespace, true);
  size_t sSize = prefs.getStringLength(nvsDeviceKey) + 1u;
  std::unique_ptr<char[]> s(new char[sSize]);
  memset(s.get(), 0, sSize);
  prefs.getString(nvsDeviceKey, s.get(), sSize);
  prefs.end();
  //Serial.printf("Settings::read: %s (%d bytes) - %s\n", nvsDeviceKey, sSize, s.get());
  return String(s.get());
}

void DeviceId::reset()
{
  Preferences prefs;
  prefs.begin(nvsDeviceIdNamespace);
  prefs.clear();
  prefs.end();

  DeviceId::init();
}
