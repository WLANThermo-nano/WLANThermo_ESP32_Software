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
#include "DisplayTft.h"
#include "Settings.h"
#include "TaskConfig.h"

#define LVGL_TICK_PERIOD 60

LV_FONT_DECLARE(Font_Gothic_A1_Medium_h21);
LV_FONT_DECLARE(Font_Nano_Temp_Limit_h16);

uint32_t DisplayTft::updateTemperature = 0u;
uint32_t DisplayTft::updatePitmaster = 0u;
SystemBase *DisplayTft::system = gSystem;
uint8_t DisplayTft::serialTimeout = 0u;
boolean DisplayTft::wifiScanInProgress = false;
int8_t DisplayTft::wifiIndex = 0u;
uint8_t DisplayTft::tempPageIndex = 0u;

TFT_eSPI DisplayTft::tft = TFT_eSPI();

DisplayTft::DisplayTft()
{
  this->disabled = false;
  this->orientation = DisplayOrientation::_0;
}

void DisplayTft::init()
{
  xTaskCreatePinnedToCore(
      DisplayTft::task,           /* Task function. */
      "DisplayTft::task",         /* String with name of task. */
      10000,                      /* Stack size in bytes. */
      this,                       /* Parameter passed as input of the task */
      TASK_PRIORITY_DISPLAY_TASK, /* Priority of the task. */
      NULL,                       /* Task handle. */
      1);                         /* CPU Core */
}

boolean DisplayTft::initDisplay()
{
  if (this->disabled)
  {
    Serial.printf("DisplayTft::init: display disabled\n");
    return true;
  }

  lv_init();

  tft.init();
  tft.setRotation(3);

  lv_disp_buf_init(&lvDispBuffer, lvBuffer, NULL, LV_HOR_RES_MAX * 10);

  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = 320;
  disp_drv.ver_res = 240;
  disp_drv.flush_cb = DisplayTft::displayFlushing;
  disp_drv.buffer = &lvDispBuffer;
  lv_disp_drv_register(&disp_drv);

  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  //indev_drv.read_cb = TODO;
  lv_indev_drv_register(&indev_drv);

  createTemperatureScreen();

  return true;
}

void DisplayTft::task(void *parameter)
{
  DisplayTft *display = (DisplayTft *)parameter;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint32_t bootScreenTimeout = 200u; // 1s
  while (bootScreenTimeout || display->system->isInitDone() != true)
  {
    vTaskDelay(10);
    if (bootScreenTimeout)
      bootScreenTimeout--;
  }

  while (display->initDisplay() == false)
    vTaskDelay(1000);

  for (;;)
  {
    display->update();
    // Wait for the next cycle.
    vTaskDelay(TASK_CYCLE_TIME_DISPLAY_TASK);
  }
}

void DisplayTft::update()
{
  static uint8_t updateInProgress = false;
  static boolean wakeup = false;

  if (this->disabled)
    return;

  if (gSystem->otaUpdate.isUpdateInProgress())
  {
    if (false == updateInProgress)
    {
      updateInProgress = true;
    }
    return;
  }

  static uint32_t lastMillis = 0u;
  if ((millis() - lastMillis) >= 1000u)
  {
    lastMillis = millis();

    for (uint8_t i = 0u; (i < DISPLAY_TFT_TEMPERATURES_PER_PAGE) && (i < system->temperatures.count()); i++)
    {
      lvTemperatureTileType *tile = &lvTemperatureTiles[i];
      char currentString[10] = "OFF";
      if (system->temperatures[i]->isActive())
      {
        sprintf(currentString, "%.1lf°%c", system->temperatures[i]->getValue(), (char)system->temperatures.getUnit());
      }

      //lv_event_send(tile->objTile, LV_EVENT_REFRESH, tile);
      lv_label_set_text(tile->labelCurrent, currentString);
    }
  }

  lv_tick_inc(TASK_CYCLE_TIME_DISPLAY_TASK);
  lv_task_handler();
}

void DisplayTft::calibrate()
{
}

void DisplayTft::displayFlushing(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors(&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

void DisplayTft::createTemperatureScreen()
{
  lv_obj_t *contHeader = lv_cont_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_bg_color(contHeader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_obj_set_style_local_bg_grad_color(contHeader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_obj_set_style_local_border_width(contHeader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_clip_corner(contHeader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, false);
  lv_obj_set_style_local_radius(contHeader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_bottom(contHeader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_inner(contHeader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_left(contHeader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_right(contHeader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_top(contHeader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_bottom(contHeader, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_click(contHeader, false);
  lv_obj_set_size(contHeader, 320, 37);
  lv_cont_set_layout(contHeader, LV_LAYOUT_PRETTY_TOP);

  lv_obj_t *contTemperature = lv_cont_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_bg_color(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_obj_set_style_local_bg_grad_color(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_obj_set_style_local_border_width(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_clip_corner(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, false);
  lv_obj_set_style_local_radius(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_bottom(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_inner(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_left(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_right(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_top(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_bottom(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_click(contTemperature, false);
  lv_obj_set_size(contTemperature, 320, 203);
  lv_obj_set_pos(contTemperature, 0, 37);
  lv_cont_set_layout(contTemperature, LV_LAYOUT_PRETTY_TOP);

  for (uint8_t i = 0u; i < DISPLAY_TFT_TEMPERATURES_PER_PAGE; i++)
  {
    lvTemperatureTileType *tile = &lvTemperatureTiles[i];

    tile->objTile = lv_obj_create(contTemperature, NULL);
    lv_obj_set_style_local_bg_color(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x4A, 0x4A, 0x4A));
    lv_obj_set_style_local_bg_grad_color(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x4A, 0x4A, 0x4A));
    lv_obj_set_style_local_border_width(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_clip_corner(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, false);
    lv_obj_set_style_local_radius(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_text_color(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->objTile, 156, 63);
    lv_obj_set_event_cb(tile->objTile, DisplayTft::temperatureTileEvent);

    tile->objColor = lv_obj_create(tile->objTile, NULL);
    lv_obj_set_style_local_bg_color(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0x00, 0x00));
    lv_obj_set_style_local_bg_grad_color(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0x00, 0x00));
    lv_obj_set_style_local_border_width(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_clip_corner(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, false);
    lv_obj_set_style_local_radius(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_size(tile->objColor, 10, 63);
    lv_obj_set_pos(tile->objColor, 0, 0);

    tile->labelName = lv_label_create(tile->objTile, NULL);
    lv_label_set_text(tile->labelName, system->temperatures[i]->getName().c_str());
    lv_obj_set_style_local_text_color(tile->labelName, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelName, 109, 21);
    lv_obj_set_pos(tile->labelName, 15, 0);

    tile->labelNumber = lv_label_create(tile->objTile, NULL);
    lv_label_set_align(tile->labelNumber, LV_LABEL_ALIGN_RIGHT);
    lv_label_set_long_mode(tile->labelNumber, LV_LABEL_LONG_BREAK);
    lv_label_set_text_fmt(tile->labelNumber, "#%d", i + 1u);
    lv_obj_set_style_local_text_color(tile->labelNumber, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelNumber, 28, 21);
    lv_obj_set_pos(tile->labelNumber, 124, 0);

    tile->labelSymbolMax = lv_label_create(tile->objTile, NULL);
    lv_label_set_text(tile->labelSymbolMax, "F");
    lv_obj_set_style_local_text_font(tile->labelSymbolMax, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Nano_Temp_Limit_h16);
    lv_obj_set_style_local_text_color(tile->labelSymbolMax, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelSymbolMax, 20, 21);
    lv_obj_set_pos(tile->labelSymbolMax, 15, 21);

    tile->labelMax = lv_label_create(tile->objTile, NULL);
    lv_label_set_align(tile->labelMax, LV_LABEL_ALIGN_RIGHT);
    lv_label_set_long_mode(tile->labelMax, LV_LABEL_LONG_BREAK);
    lv_obj_set_style_local_text_color(tile->labelMax, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_label_set_text_fmt(tile->labelMax, "%i°", (int)system->temperatures[i]->getMaxValue());
    lv_obj_set_size(tile->labelMax, 30, 21);
    lv_obj_set_pos(tile->labelMax, 35, 21);

    tile->labelSymbolMin = lv_label_create(tile->objTile, NULL);
    lv_label_set_text(tile->labelSymbolMin, "E");
    lv_obj_set_style_local_text_font(tile->labelSymbolMin, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Nano_Temp_Limit_h16);
    lv_obj_set_style_local_text_color(tile->labelSymbolMin, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelSymbolMin, 20, 21);
    lv_obj_set_pos(tile->labelSymbolMin, 15, 42);

    tile->labelMin = lv_label_create(tile->objTile, NULL);
    lv_label_set_align(tile->labelMin, LV_LABEL_ALIGN_RIGHT);
    lv_label_set_long_mode(tile->labelMin, LV_LABEL_LONG_BREAK);
    lv_obj_set_style_local_text_color(tile->labelMin, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_label_set_text_fmt(tile->labelMin, "%i°", (int)system->temperatures[i]->getMinValue());
    lv_obj_set_size(tile->labelMin, 30, 21);
    lv_obj_set_pos(tile->labelMin, 35, 40);

    tile->labelCurrent = lv_label_create(tile->objTile, NULL);
    lv_label_set_align(tile->labelCurrent, LV_LABEL_ALIGN_RIGHT);
    lv_label_set_long_mode(tile->labelCurrent, LV_LABEL_LONG_BREAK);

    char currentString[10] = "OFF";
    if (system->temperatures[i]->isActive())
    {
      sprintf(currentString, "%.1lf°%c", system->temperatures[i]->getValue(), (char)system->temperatures.getUnit());
    }

    lv_label_set_text(tile->labelCurrent, currentString);
    lv_obj_set_style_local_text_font(tile->labelCurrent, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Gothic_A1_Medium_h21);
    lv_obj_set_style_local_text_color(tile->labelCurrent, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelCurrent, 82, 42);
    lv_obj_set_pos(tile->labelCurrent, 71, 37);
  }
}

void DisplayTft::temperatureTileEvent(lv_obj_t *obj, lv_event_t event)
{
  switch (event)
  {
  case LV_EVENT_REFRESH:
    break;
  }
}