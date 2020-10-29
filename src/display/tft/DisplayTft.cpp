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
#include "Preferences.h"

#define UPDATE_ALL 0xFFFFFFFFu
#define TFT_TOUCH_CALIBRATION_ARRAY_SIZE 5u

LV_FONT_DECLARE(Font_Gothic_A1_Medium_h16);
LV_FONT_DECLARE(Font_Gothic_A1_Medium_h21);
LV_FONT_DECLARE(Font_Nano_Temp_Limit_h16);
LV_FONT_DECLARE(Font_Nano_h24);

static const char *wifiSymbolText[4] = {"I", "H", "G", ""};

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

  // configure dimming IC
  pca9533.init();
  Serial.println("Setup LED Controller:");
  Serial.println(pca9533.ping());
  pca9533.setPSC(REG_PSC0, 0);
  pca9533.setPSC(REG_PSC1, 29);
  pca9533.setMODE(IO0, LED_MODE_ON);
  pca9533.setMODE(IO1, LED_MODE_ON);
  pca9533.setMODE(IO2, LED_MODE_ON);
  pca9533.setMODE(IO3, LED_MODE_ON);

  // configure PIN mode
  pinMode(TFT_CS, OUTPUT);
  pinMode(TOUCH_CS, OUTPUT);

  // set initial PIN state
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(TOUCH_CS, HIGH);

  lv_init();

  tft.init();
  tft.setRotation(1);

  calibrate();

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
  indev_drv.read_cb = DisplayTft::touchRead;
  lv_indev_drv_register(&indev_drv);

  createTemperatureScreen();
  updateTemperatureScreenSymbols(true);

  // register for all temperature callbacks
  system->temperatures.registerCallback(temperatureUpdateCb, this);

  return true;
}

void DisplayTft::calibrate()
{
  Preferences prefs;
  uint16_t touchCalibration[TFT_TOUCH_CALIBRATION_ARRAY_SIZE];
  size_t touchCalibrationSize;

  prefs.begin("TFT", true);
  touchCalibrationSize = prefs.getBytes("Touch", touchCalibration, sizeof(uint16_t) * TFT_TOUCH_CALIBRATION_ARRAY_SIZE);
  prefs.end();

  if (((sizeof(uint16_t) * TFT_TOUCH_CALIBRATION_ARRAY_SIZE) == touchCalibrationSize))
  {
    tft.setTouch(touchCalibration);
  }
  else
  {
    tft.fillScreen((0xFFFF));

    tft.setCursor(20, 0, 2);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setTextSize(1);
    tft.println("calibration run");

    tft.calibrateTouch(touchCalibration, TFT_RED, TFT_BLACK, 15);

    prefs.begin("TFT", false);
    touchCalibrationSize = prefs.putBytes("Touch", touchCalibration, sizeof(uint16_t) * TFT_TOUCH_CALIBRATION_ARRAY_SIZE);
    prefs.end();
  }
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
    vTaskDelay(TASK_CYCLE_TIME_DISPLAY_FAST_TASK);
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

  updateTemperatureScreenTiles(false);
  updateTemperatureScreenSymbols(false);

  lv_tick_inc(TASK_CYCLE_TIME_DISPLAY_FAST_TASK);
  lv_task_handler();
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

bool DisplayTft::touchRead(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  uint16_t touchX, touchY;

  bool touched = tft.getTouch(&touchX, &touchY);

  if (!touched)
  {
    return false;
  }

  if (touchX > 320 || touchY > 240)
  {
    Serial.printf("Touch coordinates issue: x: %d, y: %d\n", touchX, touchY);
  }
  else
  {

    data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;

    data->point.x = touchX;
    data->point.y = touchY;
  }

  return false;
}

void DisplayTft::createTemperatureScreen()
{
  /* create style for symbols */
  lvSymbols.style = new lv_style_t();
  lv_style_init(lvSymbols.style);
  lv_style_set_bg_color(lvSymbols.style, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_style_set_bg_grad_color(lvSymbols.style, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_style_set_border_width(lvSymbols.style, LV_STATE_DEFAULT, 0);
  lv_style_set_clip_corner(lvSymbols.style, LV_STATE_DEFAULT, false);
  lv_style_set_radius(lvSymbols.style, LV_STATE_DEFAULT, 0);
  lv_style_set_value_font(lvSymbols.style, LV_STATE_DEFAULT, &Font_Nano_h24);
  lv_style_set_value_color(lvSymbols.style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_value_align(lvSymbols.style, LV_STATE_DEFAULT, LV_ALIGN_CENTER);

  /* create container for symbols */
  static lv_obj_t *contHeader = lv_cont_create(lv_scr_act(), NULL);
  lv_obj_add_style(contHeader, LV_CONT_PART_MAIN, lvSymbols.style);
  lv_obj_set_click(contHeader, false);
  lv_obj_set_size(contHeader, 320, 40);

  /* create menu symbol */
  lvSymbols.btnMenu = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvSymbols.btnMenu, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvSymbols.btnMenu, LV_CONT_PART_MAIN, lvSymbols.style);
  lv_obj_set_style_local_value_str(lvSymbols.btnMenu, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "f");
  lv_obj_set_size(lvSymbols.btnMenu, 40, 40);
  lv_obj_set_pos(lvSymbols.btnMenu, 0, 0);

  /* create left navigation symbol */
  lvSymbols.btnLeft = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvSymbols.btnLeft, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvSymbols.btnLeft, LV_CONT_PART_MAIN, lvSymbols.style);
  lv_obj_set_style_local_value_str(lvSymbols.btnLeft, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "S");
  lv_obj_set_size(lvSymbols.btnLeft, 40, 40);
  lv_obj_set_pos(lvSymbols.btnLeft, 40, 0);
  lv_obj_set_event_cb(lvSymbols.btnLeft, DisplayTft::temperatureNavigationLeftEvent);

  /* create right navigation symbol */
  lvSymbols.btnRight = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvSymbols.btnRight, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvSymbols.btnRight, LV_CONT_PART_MAIN, lvSymbols.style);
  lv_obj_set_style_local_value_str(lvSymbols.btnRight, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "Q");
  lv_obj_set_size(lvSymbols.btnRight, 40, 40);
  lv_obj_set_pos(lvSymbols.btnRight, 80, 0);
  lv_obj_set_event_cb(lvSymbols.btnRight, DisplayTft::temperatureNavigationRightEvent);

  /* create alarm symbol */
  lvSymbols.btnAlarm = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvSymbols.btnAlarm, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvSymbols.btnAlarm, LV_CONT_PART_MAIN, lvSymbols.style);
  lv_obj_set_style_local_value_str(lvSymbols.btnAlarm, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "O");
  lv_obj_set_style_local_value_color(lvSymbols.btnAlarm, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0xFF, 0x00, 0x00));
  lv_obj_set_size(lvSymbols.btnAlarm, 40, 40);
  lv_obj_set_pos(lvSymbols.btnAlarm, 200, 0);

  /* create cloud symbol */
  lvSymbols.btnCloud = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvSymbols.btnCloud, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvSymbols.btnCloud, LV_CONT_PART_MAIN, lvSymbols.style);
  lv_obj_set_style_local_value_str(lvSymbols.btnCloud, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "O");
  lv_obj_set_style_local_value_color(lvSymbols.btnCloud, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x00, 0xFF, 0x00));
  lv_obj_set_size(lvSymbols.btnCloud, 40, 40);
  lv_obj_set_pos(lvSymbols.btnCloud, 240, 0);

  /* create wifi symbol */
  lvSymbols.btnWifi = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvSymbols.btnWifi, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvSymbols.btnWifi, LV_CONT_PART_MAIN, lvSymbols.style);
  lv_obj_set_style_local_value_str(lvSymbols.btnWifi, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
  lv_obj_set_size(lvSymbols.btnWifi, 40, 40);
  lv_obj_set_pos(lvSymbols.btnWifi, 280, 0);
  lv_obj_set_hidden(lvSymbols.btnWifi, true);

  static lv_obj_t *contTemperature = lv_cont_create(lv_scr_act(), NULL);
  lv_obj_set_style_local_bg_color(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_obj_set_style_local_border_width(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_clip_corner(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, false);
  lv_obj_set_style_local_radius(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 4);
  lv_obj_set_style_local_pad_left(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_right(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_top(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_bottom(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_click(contTemperature, false);
  lv_obj_set_size(contTemperature, 330, 200);
  lv_obj_set_pos(contTemperature, 0, 40);
  lv_cont_set_layout(contTemperature, LV_LAYOUT_GRID);

  for (uint8_t i = 0u; i < DISPLAY_TFT_TEMPERATURES_PER_PAGE; i++)
  {
    lvTemperatureTileType *tile = &lvTemperatureTiles[i];

    tile->objTile = lv_obj_create(contTemperature, NULL);
    lv_obj_set_style_local_bg_color(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x4A, 0x4A, 0x4A));
    lv_obj_set_style_local_border_width(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_clip_corner(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, false);
    lv_obj_set_style_local_radius(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_text_color(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->objTile, 156, 63);
    lv_obj_set_click(tile->objTile, true);
    //lv_obj_set_event_cb(tile->objTile, DisplayTft::temperatureTileEvent);

    tile->objColor = lv_obj_create(tile->objTile, NULL);
    lv_obj_set_style_local_bg_color(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, htmlColorToLvColor(system->temperatures[i]->getColor()));
    lv_obj_set_style_local_border_width(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_clip_corner(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, false);
    lv_obj_set_style_local_radius(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_size(tile->objColor, 10, 63);
    lv_obj_set_pos(tile->objColor, 0, 0);

    tile->labelName = lv_label_create(tile->objTile, NULL);
    lv_label_set_text(tile->labelName, system->temperatures[i]->getName().c_str());
    lv_obj_set_style_local_text_font(tile->labelName, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Gothic_A1_Medium_h16);
    lv_obj_set_style_local_text_color(tile->labelName, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelName, 109, 21);
    lv_obj_set_pos(tile->labelName, 15, 0);

    tile->labelNumber = lv_label_create(tile->objTile, NULL);
    lv_label_set_align(tile->labelNumber, LV_LABEL_ALIGN_RIGHT);
    lv_label_set_long_mode(tile->labelNumber, LV_LABEL_LONG_BREAK);
    lv_label_set_text_fmt(tile->labelNumber, "#%d", i + 1u);
    lv_obj_set_style_local_text_font(tile->labelNumber, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Gothic_A1_Medium_h16);
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
    lv_obj_set_style_local_text_font(tile->labelMax, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Gothic_A1_Medium_h16);
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
    lv_obj_set_style_local_text_font(tile->labelMin, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Gothic_A1_Medium_h16);
    lv_obj_set_style_local_text_color(tile->labelMin, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_label_set_text_fmt(tile->labelMin, "%i°", (int)system->temperatures[i]->getMinValue());
    lv_obj_set_size(tile->labelMin, 30, 21);
    lv_obj_set_pos(tile->labelMin, 35, 40);

    tile->labelCurrent = lv_label_create(tile->objTile, NULL);
    lv_label_set_align(tile->labelCurrent, LV_LABEL_ALIGN_RIGHT);
    lv_label_set_long_mode(tile->labelCurrent, LV_LABEL_LONG_BREAK);

    char labelCurrentText[10] = "OFF";

    if (system->temperatures[i]->isActive())
      sprintf(labelCurrentText, "%.1lf°%c", system->temperatures[i]->getValue(), (char)system->temperatures.getUnit());

    lv_label_set_text(tile->labelCurrent, labelCurrentText);
    lv_obj_set_style_local_text_font(tile->labelCurrent, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Gothic_A1_Medium_h21);
    lv_obj_set_style_local_text_color(tile->labelCurrent, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelCurrent, 82, 42);
    lv_obj_set_pos(tile->labelCurrent, 71, 37);
  }
}

void DisplayTft::updateTemperatureScreenTiles(boolean forceUpdate)
{
  static uint32_t activeBitsOld = 0u;

  uint8_t visibleCount = 0u;
  uint32_t activeBits = system->temperatures.getActiveBits();
  boolean updatePage = forceUpdate;
  uint32_t skippedTemperatures = 0u;

  if ((activeBits != activeBitsOld) || (UPDATE_ALL == updateTemperature) || (UPDATE_ALL == updatePitmaster))
    updatePage = true;

  if (updatePage)
  {
    uint32_t numOfTemperatures = system->temperatures.getActiveCount();
    numOfTemperatures = (0u == numOfTemperatures) ? system->temperatures.count() : numOfTemperatures;
    uint8_t numOfPages = (numOfTemperatures / DISPLAY_TFT_TEMPERATURES_PER_PAGE) + 1u;
    // check if page index is still valid
    tempPageIndex = (tempPageIndex < numOfPages) ? tempPageIndex : numOfPages - 1u;
  }

  activeBitsOld = activeBits;

  // set all active bits when no temperature is active
  activeBits = (0u == activeBits) ? ((1 << system->temperatures.count()) - 1u) : activeBits;

  for (uint8_t i = 0; (i < system->temperatures.count()) && (visibleCount < DISPLAY_TFT_TEMPERATURES_PER_PAGE); i++)
  {
    if (activeBits & (1u << i))
    {
      if (skippedTemperatures >= tempPageIndex * DISPLAY_TFT_TEMPERATURES_PER_PAGE)
      {
        lvTemperatureTileType *tile = &lvTemperatureTiles[visibleCount];

        if (updatePage)
        {
          char labelCurrentText[10] = "OFF";

          if (system->temperatures[i]->isActive())
            sprintf(labelCurrentText, "%.1lf°%c", system->temperatures[i]->getValue(), (char)system->temperatures.getUnit());

          lv_label_set_text(tile->labelCurrent, labelCurrentText);

          lv_label_set_text(tile->labelName, system->temperatures[i]->getName().c_str());
          lv_obj_set_style_local_bg_color(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, htmlColorToLvColor(system->temperatures[i]->getColor()));
          lv_label_set_text_fmt(tile->labelMax, "%i°", (int)system->temperatures[i]->getMaxValue());
          lv_label_set_text_fmt(tile->labelMin, "%i°", (int)system->temperatures[i]->getMinValue());
          lv_label_set_text_fmt(tile->labelNumber, "#%d", i + 1u);
          lv_obj_set_hidden(tile->objTile, false);
        }
        else if (updateTemperature & (1u << i))
        {
          char labelCurrentText[10] = "OFF";

          if (system->temperatures[i]->isActive())
            sprintf(labelCurrentText, "%.1lf°%c", system->temperatures[i]->getValue(), (char)system->temperatures.getUnit());

          lv_label_set_text(tile->labelCurrent, labelCurrentText);
        }

        /*if (updatePitmaster & (1u << i))
          setTemperaturePitmasterName(visibleCount, system->temperatures[i]);*/

        visibleCount++;
      }
      else
      {
        skippedTemperatures++;
      }
    }
  }

  if (updatePage)
  {
    for (uint8_t i = visibleCount; i < DISPLAY_TFT_TEMPERATURES_PER_PAGE; i++)
    {
      lvTemperatureTileType *tile = &lvTemperatureTiles[i];
      lv_obj_set_hidden(tile->objTile, true);
    }
  }

  updateTemperature = 0u;
  updatePitmaster = 0u;
}

void DisplayTft::updateTemperatureScreenSymbols(boolean forceUpdate)
{
  boolean newHasAlarm = system->temperatures.hasAlarm();
  WifiState newWifiState = system->wlan.getWifiState();
  WifiStrength newWifiStrength = system->wlan.getSignalStrength();
  const char *newWifiSymbolText = wifiSymbolText[(uint8_t)WifiStrength::None];
  char wifiSymbol = 'I';
  static uint8_t cloudState = system->cloud.state;
  static boolean hasAlarm = newHasAlarm;
  static WifiState wifiState = newWifiState;
  static WifiStrength wifiStrength = newWifiStrength;
  static uint32_t debounceWifiSymbol = millis();
  static boolean delayApSymbol = true;

  if ((cloudState != system->cloud.state) || forceUpdate)
  {
    lv_obj_set_hidden(lvSymbols.btnCloud, (system->cloud.state != 2));
    cloudState = system->cloud.state;
  }

  if ((hasAlarm != newHasAlarm) || forceUpdate)
  {
    lv_obj_set_hidden(lvSymbols.btnAlarm, !newHasAlarm);
    hasAlarm = newHasAlarm;
  }

  if (delayApSymbol && (millis() > 10000u))
  {
    forceUpdate = true;
    delayApSymbol = false;
  }

  if ((wifiStrength != newWifiStrength) || forceUpdate)
  {
    switch (newWifiStrength)
    {
    case WifiStrength::High:
    case WifiStrength::Medium:
    case WifiStrength::Low:
      newWifiSymbolText = wifiSymbolText[(uint8_t)newWifiStrength];
      break;
    }
    if ((millis() - debounceWifiSymbol) >= 1000u)
    {
      wifiStrength = newWifiStrength;
      forceUpdate = true;
      debounceWifiSymbol = millis();
    }
  }

  if ((wifiState != newWifiState) || forceUpdate)
  {
    String info;

    switch (newWifiState)
    {
    case WifiState::SoftAPNoClient:
      if (delayApSymbol)
        break;
      lv_obj_set_style_local_value_str(lvSymbols.btnWifi, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "l");
      lv_obj_set_hidden(lvSymbols.btnWifi, false);
      //NexVariable(DONT_CARE, DONT_CARE, "wifi_info.WifiName").setText(system->wlan.getAccessPointName().c_str());
      //NexVariable(DONT_CARE, DONT_CARE, "wifi_info.CustomInfo").setText("12345678");
      break;
    case WifiState::SoftAPClientConnected:
      if (delayApSymbol)
        break;
      lv_obj_set_style_local_value_str(lvSymbols.btnWifi, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "l");
      lv_obj_set_hidden(lvSymbols.btnWifi, false);
      //NexVariable(DONT_CARE, DONT_CARE, "wifi_info.WifiName").setText("");
      //NexVariable(DONT_CARE, DONT_CARE, "wifi_info.CustomInfo").setText("http://192.168.66.1");
      break;
    case WifiState::ConnectedToSTA:
      //info = "http://" + WiFi.localIP().toString();
      lv_obj_set_style_local_value_str(lvSymbols.btnWifi, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, newWifiSymbolText);
      lv_obj_set_hidden(lvSymbols.btnWifi, false);
      //NexText(DONT_CARE, DONT_CARE, "wifi_info.WifiName").setText(WiFi.SSID().c_str());
      //NexText(DONT_CARE, DONT_CARE, "wifi_info.CustomInfo").setText(info.c_str());
      break;
    case WifiState::ConnectingToSTA:
    case WifiState::AddCredentials:
      break;
    default:
      lv_obj_set_style_local_value_str(lvSymbols.btnWifi, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
      lv_obj_set_hidden(lvSymbols.btnWifi, true);
      //NexText(DONT_CARE, DONT_CARE, "wifi_info.WifiName").setText("");
      //NexText(DONT_CARE, DONT_CARE, "wifi_info.CustomInfo").setText("");
      break;
    }
    wifiState = newWifiState;
  }
}

lv_color_t DisplayTft::htmlColorToLvColor(String htmlColor)
{
  // Get rid of '#' and convert it to integer
  uint32_t number = (uint32_t)strtol(htmlColor.substring(1).c_str(), NULL, 16);

  // Split them up into r, g, b values
  uint8_t r = number >> 16;
  uint8_t g = number >> 8 & 0xFF;
  uint8_t b = number & 0xFF;

  return LV_COLOR_MAKE(r, g, b);
}

void DisplayTft::temperatureUpdateCb(uint8_t index, TemperatureBase *temperature, boolean settingsChanged, void *userData)
{
  updateTemperature |= (true == settingsChanged) ? UPDATE_ALL : (1u << index);
}

void DisplayTft::temperatureTileEvent(lv_obj_t *obj, lv_event_t event)
{
  DisplayTft *tftDisplay = (DisplayTft *)gDisplay;

  if (LV_EVENT_CLICKED == event)
  {
    lv_obj_t *win = lv_win_create(lv_scr_act(), NULL);
    lv_win_set_title(win, "Settings");
    lv_win_set_header_height(win, 40);
    lv_obj_add_style(win, LV_WIN_PART_HEADER, tftDisplay->lvSymbols.style);

    lv_obj_t *btnClose = lv_win_add_btn(win, LV_SYMBOL_CLOSE);
    lv_obj_t *btnOk = lv_win_add_btn(win, LV_SYMBOL_OK);
  }
}

void DisplayTft::temperatureNavigationLeftEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    DisplayTft *displayTft = (DisplayTft *)gDisplay;

    int8_t newPageIndex = tempPageIndex - 1;
    uint32_t numOfTemperatures = gSystem->temperatures.getActiveCount();

    numOfTemperatures = (0u == numOfTemperatures) ? gSystem->temperatures.count() : numOfTemperatures;
    uint8_t numOfPages = (numOfTemperatures / DISPLAY_TFT_TEMPERATURES_PER_PAGE) + 1u;

    if (newPageIndex < 0)
    {
      newPageIndex = numOfPages - 1u;
    }
    else if (newPageIndex >= numOfPages)
    {
      newPageIndex = 0u;
    }

    if (tempPageIndex != newPageIndex)
    {
      tempPageIndex = newPageIndex;
      displayTft->updateTemperatureScreenTiles(true);
    }
  }
}

void DisplayTft::temperatureNavigationRightEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    DisplayTft *displayTft = (DisplayTft *)gDisplay;

    int8_t newPageIndex = tempPageIndex + 1;
    uint32_t numOfTemperatures = gSystem->temperatures.getActiveCount();

    numOfTemperatures = (0u == numOfTemperatures) ? gSystem->temperatures.count() : numOfTemperatures;
    uint8_t numOfPages = (numOfTemperatures / DISPLAY_TFT_TEMPERATURES_PER_PAGE) + 1u;

    if (newPageIndex < 0)
    {
      newPageIndex = numOfPages - 1u;
    }
    else if (newPageIndex >= numOfPages)
    {
      newPageIndex = 0u;
    }

    if (tempPageIndex != newPageIndex)
    {
      tempPageIndex = newPageIndex;
      displayTft->updateTemperatureScreenTiles(true);
    }
  }
}
