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
#include "lvHome.h"
#include "lvScreen.h"
#include "lv_qrcode.h"
#include "system/SystemBase.h"
#include "display/DisplayBase.h"
#include "display/tft/DisplayTft.h"

#define UPDATE_ALL 0xFFFFFFFFu

LV_FONT_DECLARE(Font_Nano_Temp_Limit_h14);
LV_FONT_DECLARE(Font_Nano_h24);
LV_FONT_DECLARE(Font_Nano_h40);
LV_FONT_DECLARE(Font_Roboto_Medium_h22);
LV_FONT_DECLARE(Font_Roboto_Regular_h16);
LV_FONT_DECLARE(Font_Roboto_Regular_h14);

#define LV_COLOR_LIGHTBLUE LV_COLOR_MAKE(0x00, 0xBF, 0xFF)
#define LVHOME_COLOR_TILE LV_COLOR_MAKE(0x29, 0x31, 0x3A)
#define LVHOME_COLOR_BG LV_COLOR_MAKE(0x18, 0x1d, 0x23)

#define LVHOME_SYMBOL_BATTERY_PLUGGED_INDEX 7u
#define LVHOME_SYMBOL_BATTERY_CHARGING_INDEX 8u
#define LVHOME_SYMBOL_BATTERY_DISCHARGING_MAX_INDEX 6u

static const char *lvHome_WifiSymbolText[4] = {"I", "H", "G", ""};
static const char *lvHome_PitSymbols[3] = {"J", "}", "~"};
static const lv_color_t lvHome_AlarmColorMap[] = {LV_COLOR_WHITE, LV_COLOR_LIGHTBLUE, LV_COLOR_RED};
static uint32_t lvHome_UpdateTemperature = 0u;
static uint32_t lvHome_UpdatePitmaster = 0u;
static uint8_t lvHome_TempPageIndex = 0u;
static boolean lvHome_InitOnceDone = false;

static lvHomeType lvHome = {NULL};

static void lvHome_UpdateSensorTiles(boolean forceUpdate);
static void lvHome_CreateMsgBox(const char *text, lv_color_t textColor);
static void lvHome_UpdateBuzzerMsgBox(void);
static void lvHome_UpdateBatterySymbol(boolean forceUpdate);
static void lvHome_UpdatePitmasterSymbol(boolean forceUpdate);
static void lvHome_UpdateAlarmSymbol(boolean forceUpdate);
static void lvHome_UpdateSymbols(boolean forceUpdate);
static lv_color_t htmlColorToLvColor(String htmlColor);
static void lvlvHome_UpdateSensorCb(uint8_t index, TemperatureBase *temperature, boolean settingsChanged, void *userData);
static void lvHome_TileEvent(lv_obj_t *obj, lv_event_t event);
static void lvHome_NavigationMenuEvent(lv_obj_t *obj, lv_event_t event);
static void lvHome_NavigationLeftEvent(lv_obj_t *obj, lv_event_t event);
static void lvHome_NavigationRightEvent(lv_obj_t *obj, lv_event_t event);
static void lvHome_NavigationWifiEvent(lv_obj_t *obj, lv_event_t event);
static void lvHome_NavigationPitmasterEvent(lv_obj_t *obj, lv_event_t event);

void lvHome_Create(void *userData)
{
  /* create style for symbols */
  lvHome.symbols.style = new lv_style_t();
  lv_style_init(lvHome.symbols.style);
  lv_style_set_bg_color(lvHome.symbols.style, LV_STATE_DEFAULT, LVHOME_COLOR_BG);
  lv_style_set_bg_grad_color(lvHome.symbols.style, LV_STATE_DEFAULT, LVHOME_COLOR_BG);
  lv_style_set_border_width(lvHome.symbols.style, LV_STATE_DEFAULT, 0);
  lv_style_set_clip_corner(lvHome.symbols.style, LV_STATE_DEFAULT, false);
  lv_style_set_radius(lvHome.symbols.style, LV_STATE_DEFAULT, 0);
  lv_style_set_value_font(lvHome.symbols.style, LV_STATE_DEFAULT, &Font_Nano_h24);
  lv_style_set_value_color(lvHome.symbols.style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_value_align(lvHome.symbols.style, LV_STATE_DEFAULT, LV_ALIGN_CENTER);

  /* create screen for temperatures */
  lvHome.screen = lv_obj_create(NULL, NULL);

  /* create container for symbols */
  lv_obj_t *contHeader = lv_cont_create(lvHome.screen, NULL);
  lv_obj_add_style(contHeader, LV_CONT_PART_MAIN, lvHome.symbols.style);
  lv_obj_set_click(contHeader, false);
  lv_obj_set_size(contHeader, 320, 40);

  /* create menu symbol */
  lvHome.symbols.btnMenu = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvHome.symbols.btnMenu, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvHome.symbols.btnMenu, LV_CONT_PART_MAIN, lvHome.symbols.style);
  lv_obj_set_style_local_value_str(lvHome.symbols.btnMenu, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "f");
  lv_obj_set_size(lvHome.symbols.btnMenu, 40, 40);
  lv_obj_set_pos(lvHome.symbols.btnMenu, 0, 0);
  lv_obj_set_event_cb(lvHome.symbols.btnMenu, lvHome_NavigationMenuEvent);

  /* create left navigation symbol */
  lvHome.symbols.btnLeft = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvHome.symbols.btnLeft, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvHome.symbols.btnLeft, LV_CONT_PART_MAIN, lvHome.symbols.style);
  lv_obj_set_style_local_value_str(lvHome.symbols.btnLeft, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "S");
  lv_obj_set_size(lvHome.symbols.btnLeft, 40, 40);
  lv_obj_set_pos(lvHome.symbols.btnLeft, 40, 0);
  lv_obj_set_event_cb(lvHome.symbols.btnLeft, lvHome_NavigationLeftEvent);

  /* create right navigation symbol */
  lvHome.symbols.btnRight = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvHome.symbols.btnRight, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvHome.symbols.btnRight, LV_CONT_PART_MAIN, lvHome.symbols.style);
  lv_obj_set_style_local_value_str(lvHome.symbols.btnRight, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "Q");
  lv_obj_set_size(lvHome.symbols.btnRight, 40, 40);
  lv_obj_set_pos(lvHome.symbols.btnRight, 80, 0);
  lv_obj_set_event_cb(lvHome.symbols.btnRight, lvHome_NavigationRightEvent);

  /* create pitmaster symbol */
  lvHome.symbols.btnPitmaster = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvHome.symbols.btnPitmaster, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvHome.symbols.btnPitmaster, LV_CONT_PART_MAIN, lvHome.symbols.style);
  lv_obj_set_style_local_value_str(lvHome.symbols.btnPitmaster, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "J");
  lv_obj_set_size(lvHome.symbols.btnPitmaster, 40, 40);
  lv_obj_set_pos(lvHome.symbols.btnPitmaster, 160, 0);
  lv_obj_set_event_cb(lvHome.symbols.btnPitmaster, lvHome_NavigationPitmasterEvent);

  /* create alarm symbol */
  lvHome.symbols.btnAlarm = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvHome.symbols.btnAlarm, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvHome.symbols.btnAlarm, LV_CONT_PART_MAIN, lvHome.symbols.style);
  lv_obj_set_style_local_value_str(lvHome.symbols.btnAlarm, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "o");
  lv_obj_set_style_local_value_color(lvHome.symbols.btnAlarm, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_set_size(lvHome.symbols.btnAlarm, 40, 40);
  lv_obj_set_pos(lvHome.symbols.btnAlarm, 160, 0);
  lv_obj_set_hidden(lvHome.symbols.btnAlarm, true);

  /* create cloud symbol */
  lvHome.symbols.btnCloud = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvHome.symbols.btnCloud, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvHome.symbols.btnCloud, LV_CONT_PART_MAIN, lvHome.symbols.style);
  lv_obj_set_style_local_value_str(lvHome.symbols.btnCloud, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "h");
  lv_obj_set_style_local_value_color(lvHome.symbols.btnCloud, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_LIME);
  lv_obj_set_size(lvHome.symbols.btnCloud, 40, 40);
  lv_obj_set_pos(lvHome.symbols.btnCloud, 200, 0);

  /* create battery symbol */
  lvHome.symbols.btnBattery = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvHome.symbols.btnBattery, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvHome.symbols.btnBattery, LV_CONT_PART_MAIN, lvHome.symbols.style);
  lv_obj_set_style_local_value_str(lvHome.symbols.btnBattery, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
  lv_obj_set_size(lvHome.symbols.btnBattery, 40, 40);
  lv_obj_set_pos(lvHome.symbols.btnBattery, 240, 0);

  /* create wifi symbol */
  lvHome.symbols.btnWifi = lv_btn_create(contHeader, NULL);
  lv_obj_add_protect(lvHome.symbols.btnWifi, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvHome.symbols.btnWifi, LV_CONT_PART_MAIN, lvHome.symbols.style);
  lv_obj_set_style_local_value_str(lvHome.symbols.btnWifi, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
  lv_obj_set_size(lvHome.symbols.btnWifi, 40, 40);
  lv_obj_set_pos(lvHome.symbols.btnWifi, 280, 0);
  lv_obj_set_hidden(lvHome.symbols.btnWifi, true);
  lv_obj_set_event_cb(lvHome.symbols.btnWifi, lvHome_NavigationWifiEvent);

  lv_obj_t *contTemperature = lv_cont_create(lvHome.screen, NULL);
  lv_obj_set_style_local_bg_color(contTemperature, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LVHOME_COLOR_BG);
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

  for (uint8_t i = 0u; (i < LV_HOME_SENSORS_PER_PAGE) && (i < gSystem->temperatures.count()); i++)
  {
    lvHomeSensorTileType *tile = &lvHome.sensorTiles[i];

    tile->objTile = lv_obj_create(contTemperature, NULL);
    lv_obj_set_style_local_bg_color(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LVHOME_COLOR_TILE);
    lv_obj_set_style_local_border_width(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_clip_corner(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, false);
    lv_obj_set_style_local_radius(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_style_local_text_color(tile->objTile, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->objTile, 156, 63);
    lv_obj_set_click(tile->objTile, true);
    lv_obj_set_user_data(tile->objTile, gSystem->temperatures[i]);
    lv_obj_set_event_cb(tile->objTile, lvHome_TileEvent);

    tile->objColor = lv_obj_create(tile->objTile, NULL);
    lv_obj_set_style_local_bg_color(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, htmlColorToLvColor(gSystem->temperatures[i]->getColor()));
    lv_obj_set_style_local_border_width(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_clip_corner(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, false);
    lv_obj_set_style_local_radius(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
    lv_obj_set_size(tile->objColor, 10, 63);
    lv_obj_set_pos(tile->objColor, 0, 0);

    tile->labelName = lv_label_create(tile->objTile, NULL);
    lv_label_set_text(tile->labelName, gSystem->temperatures[i]->getName().c_str());
    lv_obj_set_style_local_text_font(tile->labelName, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Regular_h16);
    lv_obj_set_style_local_text_color(tile->labelName, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelName, 109, 21);
    lv_obj_set_pos(tile->labelName, 15, 1);

    tile->labelNumber = lv_label_create(tile->objTile, NULL);
    lv_label_set_align(tile->labelNumber, LV_LABEL_ALIGN_RIGHT);
    lv_label_set_long_mode(tile->labelNumber, LV_LABEL_LONG_BREAK);
    lv_label_set_text_fmt(tile->labelNumber, "#%d", i + 1u);
    lv_obj_set_style_local_text_font(tile->labelNumber, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Regular_h16);
    lv_obj_set_style_local_text_color(tile->labelNumber, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelNumber, 40, 21);
    lv_obj_set_pos(tile->labelNumber, 112, 1);

    tile->labelSymbolMax = lv_label_create(tile->objTile, NULL);
    lv_label_set_text(tile->labelSymbolMax, "F");
    lv_obj_set_style_local_text_font(tile->labelSymbolMax, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Nano_Temp_Limit_h14);
    lv_obj_set_style_local_text_color(tile->labelSymbolMax, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelSymbolMax, 20, 21);
    lv_obj_set_pos(tile->labelSymbolMax, 15, 24);

    tile->labelMax = lv_label_create(tile->objTile, NULL);
    lv_label_set_align(tile->labelMax, LV_LABEL_ALIGN_LEFT);
    lv_label_set_long_mode(tile->labelMax, LV_LABEL_LONG_BREAK);
    lv_obj_set_style_local_text_font(tile->labelMax, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Regular_h14);
    lv_obj_set_style_local_text_color(tile->labelMax, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_label_set_text_fmt(tile->labelMax, "%i°", (int)gSystem->temperatures[i]->getMaxValue());
    lv_obj_set_size(tile->labelMax, 37, 21);
    lv_obj_set_pos(tile->labelMax, 34, 22);

    tile->labelSymbolMin = lv_label_create(tile->objTile, NULL);
    lv_label_set_text(tile->labelSymbolMin, "E");
    lv_obj_set_style_local_text_font(tile->labelSymbolMin, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Nano_Temp_Limit_h14);
    lv_obj_set_style_local_text_color(tile->labelSymbolMin, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelSymbolMin, 20, 21);
    lv_obj_set_pos(tile->labelSymbolMin, 15, 44);

    tile->labelMin = lv_label_create(tile->objTile, NULL);
    lv_label_set_align(tile->labelMin, LV_LABEL_ALIGN_LEFT);
    lv_label_set_long_mode(tile->labelMin, LV_LABEL_LONG_BREAK);
    lv_obj_set_style_local_text_font(tile->labelMin, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Regular_h14);
    lv_obj_set_style_local_text_color(tile->labelMin, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_label_set_text_fmt(tile->labelMin, "%i°", (int)gSystem->temperatures[i]->getMinValue());
    lv_obj_set_size(tile->labelMin, 37, 21);
    lv_obj_set_pos(tile->labelMin, 34, 42);
    /*
    tile->labelSymbolBLE = lv_label_create(tile->objTile, NULL);
    lv_label_set_text(tile->labelSymbolBLE, "B");
    lv_obj_set_style_local_text_font(tile->labelSymbolBLE, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Regular_h16);
    lv_obj_set_style_local_text_color(tile->labelSymbolBLE, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelSymbolBLE, 20, 21);
    lv_obj_set_pos(tile->labelSymbolBLE, 109, 1);
*/
    tile->labelCurrent = lv_label_create(tile->objTile, NULL);
    lv_label_set_align(tile->labelCurrent, LV_LABEL_ALIGN_RIGHT);
    lv_label_set_long_mode(tile->labelCurrent, LV_LABEL_LONG_BREAK);

    char labelCurrentText[10] = "OFF";

    if (gSystem->temperatures[i]->isActive())
      sprintf(labelCurrentText, "%.1lf°%c", gSystem->temperatures[i]->getValue(), (char)gSystem->temperatures.getUnit());

    lv_label_set_text(tile->labelCurrent, labelCurrentText);
    lv_obj_set_style_local_text_font(tile->labelCurrent, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Medium_h22);
    lv_obj_set_style_local_text_color(tile->labelCurrent, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(tile->labelCurrent, 82, 42);
    lv_obj_set_pos(tile->labelCurrent, 70, 33);
  }

  lvHome_UpdateSensorTiles(true);
  lvHome_UpdateSymbols(true);

  if (false == lvHome_InitOnceDone)
  {
    lvHome_InitOnceDone = true;
    // register for all temperature callbacks
    gSystem->temperatures.registerCallback(lvlvHome_UpdateSensorCb, NULL);
  }

  lv_scr_load(lvHome.screen);
}

void lvHome_Update(boolean forceUpdate)
{
  lvHome_UpdateSymbols(forceUpdate);
  lvHome_UpdateSensorTiles(forceUpdate);
}

void lvHome_Delete(void)
{
  if (lvHome.screen)
  {
    lv_obj_del(lvHome.screen);
    lvHome.screen = NULL;

    delete lvHome.symbols.style;
    lvHome.symbols.style = NULL;
  }
}

void lvHome_UpdateSensorTiles(boolean forceUpdate)
{
  static uint32_t activeBitsOld = 0u;

  uint8_t visibleCount = 0u;
  uint32_t activeBits = gSystem->temperatures.getActiveBits();
  boolean updatePage = forceUpdate;
  uint32_t skippedTemperatures = 0u;

  if ((activeBits != activeBitsOld) || (UPDATE_ALL == lvHome_UpdateTemperature) || (UPDATE_ALL == lvHome_UpdatePitmaster))
    updatePage = true;

  if (updatePage)
  {
    uint32_t numOfTemperatures = gSystem->temperatures.getActiveCount();
    numOfTemperatures = (0u == numOfTemperatures) ? gSystem->temperatures.count() : numOfTemperatures;
    uint8_t numOfPages = (numOfTemperatures / LV_HOME_SENSORS_PER_PAGE) + 1u;
    // check if page index is still valid
    lvHome_TempPageIndex = (lvHome_TempPageIndex < numOfPages) ? lvHome_TempPageIndex : numOfPages - 1u;
  }

  activeBitsOld = activeBits;

  // set all active bits when no temperature is active
  activeBits = (0u == activeBits) ? ((1 << gSystem->temperatures.count()) - 1u) : activeBits;

  for (uint8_t i = 0; (i < gSystem->temperatures.count()) && (visibleCount < LV_HOME_SENSORS_PER_PAGE); i++)
  {
    if (activeBits & (1u << i))
    {
      if (skippedTemperatures >= lvHome_TempPageIndex * LV_HOME_SENSORS_PER_PAGE)
      {
        lvHomeSensorTile *tile = &lvHome.sensorTiles[visibleCount];

        if (updatePage)
        {
          char labelCurrentText[10] = "OFF";

          if (gSystem->temperatures[i]->isActive())
            sprintf(labelCurrentText, "%.1lf°%c", gSystem->temperatures[i]->getValue(), (char)gSystem->temperatures.getUnit());

          lv_label_set_text(tile->labelCurrent, labelCurrentText);
          lv_obj_set_style_local_text_color(tile->labelCurrent, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lvHome_AlarmColorMap[gSystem->temperatures[i]->getAlarmStatus()]);

          lv_label_set_text(tile->labelName, gSystem->temperatures[i]->getName().c_str());
          lv_obj_set_style_local_bg_color(tile->objColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, htmlColorToLvColor(gSystem->temperatures[i]->getColor()));
          lv_label_set_text_fmt(tile->labelMax, "%i°", (int)gSystem->temperatures[i]->getMaxValue());
          lv_label_set_text_fmt(tile->labelMin, "%i°", (int)gSystem->temperatures[i]->getMinValue());
          lv_label_set_text_fmt(tile->labelNumber, "#%d", i + 1u);
          lv_obj_set_user_data(tile->objTile, gSystem->temperatures[i]);
          lv_obj_set_hidden(tile->objTile, false);
        }
        else if (lvHome_UpdateTemperature & (1u << i))
        {
          char labelCurrentText[10] = "OFF";

          if (gSystem->temperatures[i]->isActive())
            sprintf(labelCurrentText, "%.1lf°%c", gSystem->temperatures[i]->getValue(), (char)gSystem->temperatures.getUnit());

          lv_label_set_text(tile->labelCurrent, labelCurrentText);
          lv_obj_set_style_local_text_color(tile->labelCurrent, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, lvHome_AlarmColorMap[gSystem->temperatures[i]->getAlarmStatus()]);
        }

        /*if (lvHome_UpdatePitmaster & (1u << i))
          setTemperaturePitmasterName(visibleCount, gSystem->temperatures[i]);*/

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
    for (uint8_t i = visibleCount; i < LV_HOME_SENSORS_PER_PAGE; i++)
    {
      lvHomeSensorTile *tile = &lvHome.sensorTiles[i];
      lv_obj_set_hidden(tile->objTile, true);
    }
  }

  lvHome_UpdateTemperature = 0u;
  lvHome_UpdatePitmaster = 0u;
}

void lvHome_CreateMsgBox(const char *text, lv_color_t textColor)
{
  static const char *btns[] = {"OK", ""};

  lv_obj_t *mbox = lv_msgbox_create(lvHome.screen, NULL);
  lv_msgbox_set_text(mbox, "o");
  lv_msgbox_ext_t *ext = (lv_msgbox_ext_t *)lv_obj_get_ext_attr(mbox);
  lv_obj_set_style_local_text_font(ext->text, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Nano_h40);
  lv_obj_set_style_local_text_color(ext->text, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, textColor);

  lv_msgbox_add_btns(mbox, btns);
  lv_obj_set_width(mbox, 200);
  lv_obj_align(mbox, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_event_cb(mbox,
                      [](lv_obj_t *obj, lv_event_t event) 
                      {
                        if (event == LV_EVENT_VALUE_CHANGED)
                        {
                          const char *buttonText = lv_msgbox_get_text(obj);

                          if (0u == strcmp(buttonText, "o"))
                          {
                            gSystem->temperatures.acknowledgeAlarm();
                            gSystem->getBuzzer()->disable();
                            lv_msgbox_start_auto_close(obj, 0);
                          }
                        }
                      });
}

void lvHome_UpdateBuzzerMsgBox(void)
{
  boolean newBuzzerEnabled = gSystem->getBuzzer()->isEnabled();
  static boolean BuzzerEnabled = false;

  if (BuzzerEnabled != newBuzzerEnabled)
  {
    if (true == newBuzzerEnabled)
    {
      lvHome_CreateMsgBox("o", LV_COLOR_RED);
    }

    BuzzerEnabled = newBuzzerEnabled;
  }
}

void lvHome_UpdateBatterySymbol(boolean forceUpdate)
{
  const char *batterySymbolText[9] = {"v", "w", "x", "y", "z", "{", "|", "s", "u"};
  const char *newBatterySymbol;

  if (true == gSystem->battery->isCharging())
  {
    newBatterySymbol = batterySymbolText[LVHOME_SYMBOL_BATTERY_CHARGING_INDEX];
  }
  else if (true == gSystem->battery->isUsbPowered())
  {
    newBatterySymbol = batterySymbolText[LVHOME_SYMBOL_BATTERY_PLUGGED_INDEX];
  }
  else
  {
    long symbolIndex = map(gSystem->battery->percentage, 0, 100, 0, LVHOME_SYMBOL_BATTERY_DISCHARGING_MAX_INDEX);
    newBatterySymbol = batterySymbolText[symbolIndex];
  }

  static const char *batterySymbol = newBatterySymbol;

  if ((batterySymbol != newBatterySymbol) || forceUpdate)
  {
    lv_obj_set_style_local_value_str(lvHome.symbols.btnBattery, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, newBatterySymbol);
    batterySymbol = newBatterySymbol;
  }
}

void lvHome_UpdatePitmasterSymbol(boolean forceUpdate)
{
  static uint8_t angle = 0u;
  static uint32_t counter = 0u;

  boolean pitmasterIsRunning = false;
  boolean pitmasterShouldSpin = false;
  boolean pitmasterIsEnabled = gSystem->pitmasters.isEnabled();

  for (uint8_t index = 0u; index < gSystem->pitmasters.count(); index++)
  {
    if (gSystem->pitmasters[index]->getType() != pm_off)
    {
      pitmasterIsRunning = true;
      pitmasterShouldSpin = (gSystem->pitmasters[index]->getValue() > 0u) && pitmasterIsEnabled;
      break;
    }
  }

  if (pitmasterIsRunning)
  {
    if (pitmasterShouldSpin)
    {
      // do animation
      if ((counter++ % 10u) == 0u)
      {
        lv_obj_set_style_local_value_str(lvHome.symbols.btnPitmaster, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lvHome_PitSymbols[angle]);
        angle = (angle >= 2u) ? 0u : (angle + 1u);
      }
    }
    else
    {
      // stop animation
      angle = 0u;
      lv_obj_set_style_local_value_str(lvHome.symbols.btnPitmaster, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lvHome_PitSymbols[angle]);
    }
  }
  else
  {
    lv_obj_set_style_local_value_str(lvHome.symbols.btnPitmaster, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
  }
}

void lvHome_UpdateAlarmSymbol(boolean forceUpdate)
{
  boolean newHasAlarm = gSystem->temperatures.hasAlarm(false);
  static boolean hasAlarm = false;

  if ((hasAlarm != newHasAlarm) || forceUpdate)
  {
    lv_obj_set_hidden(lvHome.symbols.btnAlarm, !newHasAlarm);
    hasAlarm = newHasAlarm;
  }
}

void lvHome_UpdateSymbols(boolean forceUpdate)
{
  WifiState newWifiState = gSystem->wlan.getWifiState();
  WifiStrength newWifiStrength = gSystem->wlan.getSignalStrength();
  const char *newWifiSymbolText = lvHome_WifiSymbolText[(uint8_t)WifiStrength::None];
  char wifiSymbol = 'I';
  static uint8_t cloudState = gSystem->cloud.state;
  static WifiState wifiState = newWifiState;
  static WifiStrength wifiStrength = newWifiStrength;
  static uint32_t debounceWifiSymbol = millis();
  static boolean delayApSymbol = true;

  lvHome_UpdateBatterySymbol(forceUpdate);
  lvHome_UpdatePitmasterSymbol(forceUpdate);
  lvHome_UpdateAlarmSymbol(forceUpdate);
  lvHome_UpdateBuzzerMsgBox();

  if ((cloudState != gSystem->cloud.state) || forceUpdate)
  {
    lv_obj_set_hidden(lvHome.symbols.btnCloud, (gSystem->cloud.state != 2));
    cloudState = gSystem->cloud.state;
  }

  if (delayApSymbol && (millis() > 30000u))
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
      newWifiSymbolText = lvHome_WifiSymbolText[(uint8_t)newWifiStrength];
      break;
    }
    if (((millis() - debounceWifiSymbol) >= 1000u) && (false == delayApSymbol))
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
      lv_obj_set_style_local_value_str(lvHome.symbols.btnWifi, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "l");
      lv_obj_set_hidden(lvHome.symbols.btnWifi, false);
      break;
    case WifiState::SoftAPClientConnected:
      if (delayApSymbol)
        break;
      lv_obj_set_style_local_value_str(lvHome.symbols.btnWifi, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "l");
      lv_obj_set_hidden(lvHome.symbols.btnWifi, false);
      break;
    case WifiState::ConnectedToSTA:
      lv_obj_set_style_local_value_str(lvHome.symbols.btnWifi, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, newWifiSymbolText);
      lv_obj_set_hidden(lvHome.symbols.btnWifi, false);
      break;
    case WifiState::ConnectingToSTA:
    case WifiState::AddCredentials:
      break;
    default:
      lv_obj_set_style_local_value_str(lvHome.symbols.btnWifi, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "");
      lv_obj_set_hidden(lvHome.symbols.btnWifi, true);
      break;
    }
    wifiState = newWifiState;
  }
}

lv_color_t htmlColorToLvColor(String htmlColor)
{
  // Get rid of '#' and convert it to integer
  uint32_t number = (uint32_t)strtol(htmlColor.substring(1).c_str(), NULL, 16);

  // Split them up into r, g, b values
  uint8_t r = number >> 16;
  uint8_t g = number >> 8 & 0xFF;
  uint8_t b = number & 0xFF;

  return LV_COLOR_MAKE(r, g, b);
}

void lvlvHome_UpdateSensorCb(uint8_t index, TemperatureBase *temperature, boolean settingsChanged, void *userData)
{
  lvHome_UpdateTemperature |= (true == settingsChanged) ? UPDATE_ALL : (1u << index);
}

void lvHome_TileEvent(lv_obj_t *obj, lv_event_t event)
{
  DisplayTft *tftDisplay = (DisplayTft *)gDisplay;

  if (LV_EVENT_CLICKED == event)
  {
    lvScreen_Open(lvScreenType::Temperature, lv_obj_get_user_data(obj));
  }
}

void lvHome_NavigationMenuEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvScreen_Open(lvScreenType::Display);
  }
}

void lvHome_NavigationLeftEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    DisplayTft *displayTft = (DisplayTft *)gDisplay;

    int8_t newPageIndex = lvHome_TempPageIndex - 1;
    uint32_t numOfTemperatures = gSystem->temperatures.getActiveCount();

    numOfTemperatures = (0u == numOfTemperatures) ? gSystem->temperatures.count() : numOfTemperatures;
    uint8_t numOfPages = (numOfTemperatures / LV_HOME_SENSORS_PER_PAGE) + 1u;

    if (newPageIndex < 0)
    {
      newPageIndex = numOfPages - 1u;
    }
    else if (newPageIndex >= numOfPages)
    {
      newPageIndex = 0u;
    }

    if (lvHome_TempPageIndex != newPageIndex)
    {
      lvHome_TempPageIndex = newPageIndex;
      lvHome_UpdateSensorTiles(true);
    }
  }
}

void lvHome_NavigationRightEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    DisplayTft *displayTft = (DisplayTft *)gDisplay;

    int8_t newPageIndex = lvHome_TempPageIndex + 1;
    uint32_t numOfTemperatures = gSystem->temperatures.getActiveCount();

    numOfTemperatures = (0u == numOfTemperatures) ? gSystem->temperatures.count() : numOfTemperatures;
    uint8_t numOfPages = (numOfTemperatures / LV_HOME_SENSORS_PER_PAGE) + 1u;

    if (newPageIndex < 0)
    {
      newPageIndex = numOfPages - 1u;
    }
    else if (newPageIndex >= numOfPages)
    {
      newPageIndex = 0u;
    }

    if (lvHome_TempPageIndex != newPageIndex)
    {
      lvHome_TempPageIndex = newPageIndex;
      lvHome_UpdateSensorTiles(true);
    }
  }
}

void lvHome_NavigationWifiEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvScreen_Open(lvScreenType::Wifi);
  }
}

void lvHome_NavigationPitmasterEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvScreen_Open(lvScreenType::Pitmaster);
  }
}