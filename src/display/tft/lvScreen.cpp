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
#include "lvMenu.h"
#include "lvDisplay.h"
#include "lvTemperature.h"
#include "lvUpdate.h"
#include "lvPitmaster.h"

static lvScreenType lvScreen_CurrentScreen = lvScreenType::None;
static lvScreenType lvScreen_RequestedScreen = lvScreenType::None;
static void *lvScreen_UserDataPtr = NULL;

static lvScreenFuncType lvScreen_Functions[] = {
    {lvScreenType::None, NULL, NULL, NULL},
    {lvScreenType::Menu, lvMenu_Create, lvMenu_Update, lvMenu_Delete},
    {lvScreenType::Home, lvHome_Create, lvHome_Update, lvHome_Delete},
    {lvScreenType::Wifi, lvWifi_Create, lvWifi_Update, lvWifi_Delete},
    {lvScreenType::Display, lvDisplay_Create, lvDisplay_Update, lvDisplay_Delete},
    {lvScreenType::Temperature, lvTemperature_Create, lvTemperature_Update, lvTemperature_Delete},
    {lvScreenType::Update, lvUpdate_Create, lvUpdate_Update, lvUpdate_Delete},
    {lvScreenType::Pitmaster, lvPitmaster_Create, lvPitmaster_Update, lvPitmaster_Delete},
    };

void lvScreen_Open(lvScreenType screen, void *userData)
{
  if (screen != lvScreen_CurrentScreen)
  {
    lvScreen_RequestedScreen = screen;
    lvScreen_UserDataPtr = userData;
  }
}

void lvScreen_Update(void)
{
  uint8_t screenIndex;

  if ((lvScreen_RequestedScreen > lvScreenType::None) &&
      (lvScreen_RequestedScreen < lvScreenType::Max))
  {
    screenIndex = (uint8_t)lvScreen_RequestedScreen;
    if (lvScreen_Functions[screenIndex].createFunc)
    {
      lvScreen_Functions[screenIndex].createFunc(lvScreen_UserDataPtr);
      lvScreen_UserDataPtr = NULL;
    }

    screenIndex = (uint8_t)lvScreen_CurrentScreen;
    if (lvScreen_Functions[screenIndex].deleteFunc)
    {
      lvScreen_Functions[screenIndex].deleteFunc();
    }

    lvScreen_CurrentScreen = lvScreen_RequestedScreen;
    lvScreen_RequestedScreen = lvScreenType::None;

    /*lv_mem_monitor_t mon;
    lv_mem_monitor(&mon);
    printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n", (int)mon.total_size - mon.free_size,
           mon.used_pct,
           mon.frag_pct,
           (int)mon.free_biggest_size);*/
  }
  else
  {
    screenIndex = (uint8_t)lvScreen_CurrentScreen;
    if (lvScreen_Functions[screenIndex].updateFunc)
    {
      lvScreen_Functions[screenIndex].updateFunc(false);
    }
  }
}
