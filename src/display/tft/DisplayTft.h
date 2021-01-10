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
#include "system/SystemBase.h"
#include "display/DisplayBase.h"
#include <lvgl.h>
#include <Ticker.h>
#include <TFT_eSPI.h>
#include "Wire.h"

class DisplayTft : public DisplayBase
{
public:
  DisplayTft();
  void init();
  void hwInit();
  void update();
  void calibrate();
  static void setBrightness(uint8_t brightness);
  static void drawCharging();

private:
  boolean initDisplay();
  boolean isCalibrated();
  static void task(void *parameter);

  static void displayFlushing(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
  static bool touchRead(lv_indev_drv_t *indev_driver, lv_indev_data_t *data);

  static TFT_eSPI tft;
  lv_disp_buf_t lvDispBuffer;
  lv_color_t lvBuffer[LV_HOR_RES_MAX * 10];
};
