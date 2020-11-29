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
#include "lvScreen.h"
#include "lvHome.h"
#include "lvWifi.h"

static lvScreenType lvScreen_CurrentScreen = lvScreenType::None;
static lvScreenType lvScreen_requestedScreen = lvScreenType::None;

void lvScreen_Open(lvScreenType screen)
{
  if (screen != lvScreen_CurrentScreen)
    lvScreen_requestedScreen = screen;
}

void lvScreen_Update()
{
  if (lvScreen_requestedScreen != lvScreenType::None)
  {
    switch (lvScreen_requestedScreen)
    {
    case lvScreenType::Home:
      Serial.println("lvHome_Create");
      lvHome_Create();
      break;
    case lvScreenType::Wifi:
      Serial.println("lvWifi_Create");
      lvWifi_Create();
      break;
    }

    switch (lvScreen_CurrentScreen)
    {
    case lvScreenType::Home:
      Serial.println("lvHome_Delete");
      lvHome_Delete();
      break;
    case lvScreenType::Wifi:
      Serial.println("lvWifi_Delete");
      lvWifi_Delete();
      break;
    }

    lvScreen_CurrentScreen = lvScreen_requestedScreen;
    lvScreen_requestedScreen = lvScreenType::None;

    lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n", (int)mon.total_size - mon.free_size,
           mon.used_pct,
           mon.frag_pct,
           (int)mon.free_biggest_size);
  }
  else
  {
    switch (lvScreen_CurrentScreen)
    {
    case lvScreenType::Home:
      lvHome_Update(false);
      break;
    case lvScreenType::Wifi:
      lvWifi_Update(false);
      break;
    }
  }
}
