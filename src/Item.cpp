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

#include "Item.h"
#include "nvs.h"
#include "Preferences.h"
#include <memory>

#define STRINGIFY(s) #s

static char *NvsKeyStrings[] =
    {
      STRINGIFY(kItem)
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

const char *Item::nvsItemNamespace = "item";

Item::Item()
{
}

void Item::write(ItemNvsKeys key, String itemString)
{
  Prefs prefs;
  prefs.begin(nvsItemNamespace, false);
  prefs.putString(NvsKeyStrings[key], itemString.c_str());
  Serial.printf("Item::write: %s - %s\n", NvsKeyStrings[key], itemString.c_str());
  prefs.end();
}

String Item::read(ItemNvsKeys key)
{
  Prefs prefs;
  prefs.begin(nvsItemNamespace, true);
  size_t sSize = prefs.getStringLength(NvsKeyStrings[key]) + 1u;
  std::unique_ptr<char[]> s(new char[sSize]);
  memset(s.get(), 0, sSize);
  prefs.getString(NvsKeyStrings[key], s.get(), sSize);
  prefs.end();
  //Serial.printf("Settings::read: %s (%d bytes) - %s\n", NvsKeyStrings[key], sSize, s.get());
  return String(s.get());
}
