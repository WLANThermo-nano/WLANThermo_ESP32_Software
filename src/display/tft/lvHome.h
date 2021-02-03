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

#include "Arduino.h"
#include <lvgl.h>

#define LV_HOME_SENSORS_PER_PAGE 6u

typedef struct lvHomeSensorTile
{
  lv_obj_t *objTile;
  lv_obj_t *objColor;
  lv_style_t objColorStyle;
  lv_obj_t *labelName;
  lv_obj_t *labelNumber;
  lv_obj_t *labelSymbolMax;
  lv_obj_t *labelMax;
  lv_obj_t *labelSymbolMin;
  lv_obj_t *labelMin;
  lv_obj_t *labelSymbolBLE;
  lv_obj_t *labelCurrent;
} lvHomeSensorTileType;

typedef struct lvSymbols
{
  lv_style_t *style;
  lv_obj_t *btnMenu;
  lv_obj_t *btnLeft;
  lv_obj_t *btnRight;
  lv_obj_t *btnAlarm;
  lv_obj_t *btnCloud;
  lv_obj_t *btnWifi;
} lvHomeSymbolsType;

typedef struct lvHome
{
  lv_obj_t *screen;
  lvHomeSymbolsType symbols;
  lvHomeSensorTileType sensorTiles[LV_HOME_SENSORS_PER_PAGE];
} lvHomeType;

void lvHome_Create(void);
void lvHome_Update(boolean forceUpdate);
void lvHome_Delete(void);