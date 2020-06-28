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
#include "system/SystemBase.h"
#include "DisplayBase.h"
#include <lvgl.h>
#include <Ticker.h>
#include <TFT_eSPI.h>

#define DISPLAY_TFT_TEMPERATURES_PER_PAGE 6u

typedef struct lvTemperatureTile
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
  lv_obj_t *labelCurrent;
} lvTemperatureTileType;

class DisplayTft : public DisplayBase
{
public:
  DisplayTft();
  void init();
  void update();
  void calibrate();

private:
  boolean initDisplay();

  static void displayFlushing(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
  void createTemperatureScreen();
  static void task(void *parameter);
  static SystemBase *system;

  static TFT_eSPI tft;
  lv_disp_buf_t lvDispBuffer;
  lv_color_t lvBuffer[LV_HOR_RES_MAX * 10];

  lvTemperatureTileType lvTemperatureTiles[DISPLAY_TFT_TEMPERATURES_PER_PAGE];
  static uint32_t updateTemperature;
  static uint32_t updatePitmaster;
  static uint8_t serialTimeout;
  static boolean wifiScanInProgress;
  static int8_t wifiIndex;
  static uint8_t tempPageIndex;
};
