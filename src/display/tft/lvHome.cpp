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

#define LVHOME_COLOR_TILE lv_color_make(0x29, 0x31, 0x3A)
#define LVHOME_COLOR_BG   lv_color_make(0x18, 0x1d, 0x23)

#define LVHOME_SYMBOL_BATTERY_PLUGGED_INDEX       7u
#define LVHOME_SYMBOL_BATTERY_CHARGING_INDEX      8u
#define LVHOME_SYMBOL_BATTERY_DISCHARGING_MAX_INDEX 6u

static const char *lvHome_WifiSymbolText[4] = {"I", "H", "G", ""};
static const char *lvHome_PitSymbols[3] = {"J", "}", "~"};
static uint32_t lvHome_UpdateTemperature = 0u;
static uint32_t lvHome_UpdatePitmaster = 0u;
static uint8_t lvHome_TempPageIndex = 0u;
static boolean lvHome_InitOnceDone = false;

static lvHomeType lvHome = {NULL};

static lv_color_t lvHome_GetAlarmColor(uint8_t status)
{
  if (status == 1u) return lv_color_make(0x00, 0xBF, 0xFF);
  if (status == 2u) return lv_color_make(0xFF, 0x00, 0x00);
  return lv_color_white();
}

static void lvHome_UpdateSensorTiles(boolean forceUpdate);
static void lvHome_CreateMsgBox(const char *text, lv_color_t textColor);
static void lvHome_UpdateBuzzerMsgBox(void);
static void lvHome_UpdateBatterySymbol(boolean forceUpdate);
static void lvHome_UpdatePitmasterSymbol(boolean forceUpdate);
static void lvHome_UpdateAlarmSymbol(boolean forceUpdate);
static void lvHome_UpdateSymbols(boolean forceUpdate);
static lv_color_t htmlColorToLvColor(String htmlColor);
static void lvlvHome_UpdateSensorCb(uint8_t index, TemperatureBase *temperature, boolean settingsChanged, void *userData);
static void lvHome_TileEvent(lv_event_t *e);
static void lvHome_NavigationMenuEvent(lv_event_t *e);
static void lvHome_NavigationLeftEvent(lv_event_t *e);
static void lvHome_NavigationRightEvent(lv_event_t *e);
static void lvHome_NavigationWifiEvent(lv_event_t *e);
static void lvHome_NavigationPitmasterEvent(lv_event_t *e);

static void lvHome_AddSymbolBtn(lv_obj_t *parent, lv_obj_t **btn, lv_obj_t **labelOut,
                                lv_style_t *style, const char *text, lv_color_t textColor,
                                int x, int y, lv_event_cb_t cb)
{
  *btn = lv_btn_create(parent);
  lv_obj_remove_flag(*btn, LV_OBJ_FLAG_CLICK_FOCUSABLE);
  lv_obj_add_style(*btn, style, 0);
  lv_obj_set_style_pad_all(*btn, 0, 0);
  lv_obj_set_style_border_width(*btn, 0, 0);
  lv_obj_set_style_shadow_width(*btn, 0, 0);
  lv_obj_set_size(*btn, 40, 40);
  lv_obj_set_pos(*btn, x, y);
  if (cb) lv_obj_add_event_cb(*btn, cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *lbl = lv_label_create(*btn);
  lv_label_set_text(lbl, text);
  lv_obj_set_style_text_font(lbl, &Font_Nano_h24, 0);
  lv_obj_set_style_text_color(lbl, textColor, 0);
  lv_obj_center(lbl);
  if (labelOut) *labelOut = lbl;
}

void lvHome_Create(void *userData)
{
  lvHome.symbols.style = new lv_style_t();
  lv_style_init(lvHome.symbols.style);
  lv_style_set_bg_color(lvHome.symbols.style, LVHOME_COLOR_BG);
  lv_style_set_border_width(lvHome.symbols.style, 0);
  lv_style_set_clip_corner(lvHome.symbols.style, false);
  lv_style_set_radius(lvHome.symbols.style, 0);
  lv_style_set_pad_all(lvHome.symbols.style, 0);

  lvHome.screen = lv_obj_create(NULL);
  lv_obj_remove_flag(lvHome.screen, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *contHeader = lv_obj_create(lvHome.screen);
  lv_obj_add_style(contHeader, lvHome.symbols.style, 0);
  lv_obj_set_style_pad_all(contHeader, 0, 0);
  lv_obj_set_style_border_width(contHeader, 0, 0);
  lv_obj_remove_flag(contHeader, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_remove_flag(contHeader, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(contHeader, 320, 40);
  lv_obj_set_pos(contHeader, 0, 0);

  lvHome_AddSymbolBtn(contHeader, &lvHome.symbols.btnMenu, NULL,
                      lvHome.symbols.style, "f", lv_color_white(),
                      0, 0, lvHome_NavigationMenuEvent);

  lvHome_AddSymbolBtn(contHeader, &lvHome.symbols.btnLeft, NULL,
                      lvHome.symbols.style, "S", lv_color_white(),
                      40, 0, lvHome_NavigationLeftEvent);

  lvHome_AddSymbolBtn(contHeader, &lvHome.symbols.btnRight, NULL,
                      lvHome.symbols.style, "Q", lv_color_white(),
                      80, 0, lvHome_NavigationRightEvent);

  lvHome_AddSymbolBtn(contHeader, &lvHome.symbols.btnPitmaster, &lvHome.symbols.labelPitmaster,
                      lvHome.symbols.style, "J", lv_color_white(),
                      160, 0, lvHome_NavigationPitmasterEvent);

  lvHome_AddSymbolBtn(contHeader, &lvHome.symbols.btnAlarm, NULL,
                      lvHome.symbols.style, "o", lv_color_make(0xFF, 0x00, 0x00),
                      160, 0, NULL);
  lv_obj_add_flag(lvHome.symbols.btnAlarm, LV_OBJ_FLAG_HIDDEN);

  lvHome_AddSymbolBtn(contHeader, &lvHome.symbols.btnCloud, NULL,
                      lvHome.symbols.style, "h", lv_color_make(0x00, 0xFF, 0x00),
                      200, 0, NULL);

  lvHome_AddSymbolBtn(contHeader, &lvHome.symbols.btnBattery, &lvHome.symbols.labelBattery,
                      lvHome.symbols.style, "", lv_color_white(),
                      240, 0, NULL);

  lvHome_AddSymbolBtn(contHeader, &lvHome.symbols.btnWifi, &lvHome.symbols.labelWifi,
                      lvHome.symbols.style, "", lv_color_white(),
                      280, 0, lvHome_NavigationWifiEvent);
  lv_obj_add_flag(lvHome.symbols.btnWifi, LV_OBJ_FLAG_HIDDEN);

  lv_obj_t *contTemperature = lv_obj_create(lvHome.screen);
  lv_obj_set_style_bg_color(contTemperature, LVHOME_COLOR_BG, 0);
  lv_obj_set_style_border_width(contTemperature, 0, 0);
  lv_obj_set_style_clip_corner(contTemperature, false, 0);
  lv_obj_set_style_radius(contTemperature, 0, 0);
  lv_obj_set_style_pad_column(contTemperature, 4, 0);
  lv_obj_set_style_pad_row(contTemperature, 4, 0);
  lv_obj_set_style_pad_left(contTemperature, 2, 0);
  lv_obj_set_style_pad_right(contTemperature, 2, 0);
  lv_obj_set_style_pad_top(contTemperature, 0, 0);
  lv_obj_set_style_pad_bottom(contTemperature, 2, 0);
  lv_obj_remove_flag(contTemperature, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_size(contTemperature, 320, 200);
  lv_obj_set_pos(contTemperature, 0, 40);
  lv_obj_set_flex_flow(contTemperature, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(contTemperature, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

  for (uint8_t i = 0u; (i < LV_HOME_SENSORS_PER_PAGE) && (i < gSystem->temperatures.count()); i++)
  {
    lvHomeSensorTileType *tile = &lvHome.sensorTiles[i];

    tile->objTile = lv_obj_create(contTemperature);
    lv_obj_set_style_bg_color(tile->objTile, LVHOME_COLOR_TILE, 0);
    lv_obj_set_style_border_width(tile->objTile, 0, 0);
    lv_obj_set_style_clip_corner(tile->objTile, false, 0);
    lv_obj_set_style_radius(tile->objTile, 10, 0);
    lv_obj_set_style_text_color(tile->objTile, lv_color_white(), 0);
    lv_obj_set_style_pad_all(tile->objTile, 0, 0);
    lv_obj_remove_flag(tile->objTile, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(tile->objTile, 156, 63);
    lv_obj_add_flag(tile->objTile, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_user_data(tile->objTile, gSystem->temperatures[i]);
    lv_obj_add_event_cb(tile->objTile, lvHome_TileEvent, LV_EVENT_CLICKED, NULL);

    tile->objColor = lv_obj_create(tile->objTile);
    lv_obj_set_style_bg_color(tile->objColor, htmlColorToLvColor(gSystem->temperatures[i]->getColor()), 0);
    lv_obj_set_style_border_width(tile->objColor, 0, 0);
    lv_obj_set_style_clip_corner(tile->objColor, false, 0);
    lv_obj_set_style_radius(tile->objColor, 10, 0);
    lv_obj_set_style_pad_all(tile->objColor, 0, 0);
    lv_obj_remove_flag(tile->objColor, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(tile->objColor, 10, 63);
    lv_obj_set_pos(tile->objColor, 0, 0);

    tile->labelName = lv_label_create(tile->objTile);
    lv_label_set_text(tile->labelName, gSystem->temperatures[i]->getName().c_str());
    lv_obj_set_style_text_font(tile->labelName, &Font_Roboto_Regular_h16, 0);
    lv_obj_set_style_text_color(tile->labelName, lv_color_white(), 0);
    lv_obj_set_size(tile->labelName, 109, 21);
    lv_obj_set_pos(tile->labelName, 15, 1);

    tile->labelNumber = lv_label_create(tile->objTile);
    lv_obj_set_style_text_align(tile->labelNumber, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_long_mode(tile->labelNumber, LV_LABEL_LONG_WRAP);
    lv_label_set_text_fmt(tile->labelNumber, "#%d", i + 1u);
    lv_obj_set_style_text_font(tile->labelNumber, &Font_Roboto_Regular_h16, 0);
    lv_obj_set_style_text_color(tile->labelNumber, lv_color_white(), 0);
    lv_obj_set_size(tile->labelNumber, 40, 21);
    lv_obj_set_pos(tile->labelNumber, 112, 1);

    tile->labelSymbolMax = lv_label_create(tile->objTile);
    lv_label_set_text(tile->labelSymbolMax, "F");
    lv_obj_set_style_text_font(tile->labelSymbolMax, &Font_Nano_Temp_Limit_h14, 0);
    lv_obj_set_style_text_color(tile->labelSymbolMax, lv_color_white(), 0);
    lv_obj_set_size(tile->labelSymbolMax, 20, 21);
    lv_obj_set_pos(tile->labelSymbolMax, 15, 24);

    tile->labelMax = lv_label_create(tile->objTile);
    lv_obj_set_style_text_align(tile->labelMax, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_long_mode(tile->labelMax, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(tile->labelMax, &Font_Roboto_Regular_h14, 0);
    lv_obj_set_style_text_color(tile->labelMax, lv_color_white(), 0);
    lv_label_set_text_fmt(tile->labelMax, "%i°", (int)gSystem->temperatures[i]->getMaxValue());
    lv_obj_set_size(tile->labelMax, 37, 21);
    lv_obj_set_pos(tile->labelMax, 34, 22);

    tile->labelSymbolMin = lv_label_create(tile->objTile);
    lv_label_set_text(tile->labelSymbolMin, "E");
    lv_obj_set_style_text_font(tile->labelSymbolMin, &Font_Nano_Temp_Limit_h14, 0);
    lv_obj_set_style_text_color(tile->labelSymbolMin, lv_color_white(), 0);
    lv_obj_set_size(tile->labelSymbolMin, 20, 21);
    lv_obj_set_pos(tile->labelSymbolMin, 15, 44);

    tile->labelMin = lv_label_create(tile->objTile);
    lv_obj_set_style_text_align(tile->labelMin, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_long_mode(tile->labelMin, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(tile->labelMin, &Font_Roboto_Regular_h14, 0);
    lv_obj_set_style_text_color(tile->labelMin, lv_color_white(), 0);
    lv_label_set_text_fmt(tile->labelMin, "%i°", (int)gSystem->temperatures[i]->getMinValue());
    lv_obj_set_size(tile->labelMin, 37, 21);
    lv_obj_set_pos(tile->labelMin, 34, 42);

    tile->labelCurrent = lv_label_create(tile->objTile);
    lv_obj_set_style_text_align(tile->labelCurrent, LV_TEXT_ALIGN_RIGHT, 0);
    lv_label_set_long_mode(tile->labelCurrent, LV_LABEL_LONG_WRAP);

    char labelCurrentText[10] = "OFF";
    if (gSystem->temperatures[i]->isActive())
      sprintf(labelCurrentText, "%.1lf°%c", gSystem->temperatures[i]->getValue(), (char)gSystem->temperatures.getUnit());

    lv_label_set_text(tile->labelCurrent, labelCurrentText);
    lv_obj_set_style_text_font(tile->labelCurrent, &Font_Roboto_Medium_h22, 0);
    lv_obj_set_style_text_color(tile->labelCurrent, lv_color_white(), 0);
    lv_obj_set_size(tile->labelCurrent, 82, 42);
    lv_obj_set_pos(tile->labelCurrent, 70, 33);
  }

  lvHome_UpdateSensorTiles(true);
  lvHome_UpdateSymbols(true);

  if (false == lvHome_InitOnceDone)
  {
    lvHome_InitOnceDone = true;
    gSystem->temperatures.registerCallback(lvlvHome_UpdateSensorCb, NULL);
  }

  lv_screen_load(lvHome.screen);
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
    lv_obj_delete(lvHome.screen);
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
    lvHome_TempPageIndex = (lvHome_TempPageIndex < numOfPages) ? lvHome_TempPageIndex : numOfPages - 1u;
  }

  activeBitsOld = activeBits;
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
          lv_obj_set_style_text_color(tile->labelCurrent, lvHome_GetAlarmColor(gSystem->temperatures[i]->getAlarmStatus()), 0);

          lv_label_set_text(tile->labelName, gSystem->temperatures[i]->getName().c_str());
          lv_obj_set_style_bg_color(tile->objColor, htmlColorToLvColor(gSystem->temperatures[i]->getColor()), 0);
          lv_label_set_text_fmt(tile->labelMax, "%i°", (int)gSystem->temperatures[i]->getMaxValue());
          lv_label_set_text_fmt(tile->labelMin, "%i°", (int)gSystem->temperatures[i]->getMinValue());
          lv_label_set_text_fmt(tile->labelNumber, "#%d", i + 1u);
          lv_obj_set_user_data(tile->objTile, gSystem->temperatures[i]);
          lv_obj_remove_flag(tile->objTile, LV_OBJ_FLAG_HIDDEN);
        }
        else if (lvHome_UpdateTemperature & (1u << i))
        {
          char labelCurrentText[10] = "OFF";
          if (gSystem->temperatures[i]->isActive())
            sprintf(labelCurrentText, "%.1lf°%c", gSystem->temperatures[i]->getValue(), (char)gSystem->temperatures.getUnit());

          lv_label_set_text(tile->labelCurrent, labelCurrentText);
          lv_obj_set_style_text_color(tile->labelCurrent, lvHome_GetAlarmColor(gSystem->temperatures[i]->getAlarmStatus()), 0);
        }

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
      lv_obj_add_flag(tile->objTile, LV_OBJ_FLAG_HIDDEN);
    }
  }

  lvHome_UpdateTemperature = 0u;
  lvHome_UpdatePitmaster = 0u;
}

void lvHome_CreateMsgBox(const char *text, lv_color_t textColor)
{
  lv_obj_t *mbox = lv_msgbox_create(lvHome.screen);
  lv_obj_t *content = lv_msgbox_get_content(mbox);
  lv_obj_t *lbl = lv_label_create(content);
  lv_label_set_text(lbl, text);
  lv_obj_set_style_text_font(lbl, &Font_Nano_h40, 0);
  lv_obj_set_style_text_color(lbl, textColor, 0);
  lv_obj_t *btn = lv_msgbox_add_footer_button(mbox, "OK");
  lv_obj_add_event_cb(btn,
    [](lv_event_t *e)
    {
      if (lv_event_get_code(e) == LV_EVENT_CLICKED)
      {
        gSystem->temperatures.acknowledgeAlarm();
        gSystem->getBuzzer()->disable();
        lv_obj_t *m = (lv_obj_t *)lv_event_get_user_data(e);
        lv_msgbox_close(m);
      }
    },
    LV_EVENT_CLICKED, mbox);
  lv_obj_set_width(mbox, 200);
  lv_obj_center(mbox);
}

void lvHome_UpdateBuzzerMsgBox(void)
{
  boolean newBuzzerEnabled = gSystem->getBuzzer()->isEnabled();
  static boolean BuzzerEnabled = false;

  if (BuzzerEnabled != newBuzzerEnabled)
  {
    if (true == newBuzzerEnabled)
    {
      lvHome_CreateMsgBox("o", lv_color_make(0xFF, 0x00, 0x00));
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
    lv_label_set_text(lvHome.symbols.labelBattery, newBatterySymbol);
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
      if ((counter++ % 10u) == 0u)
      {
        lv_label_set_text(lvHome.symbols.labelPitmaster, lvHome_PitSymbols[angle]);
        angle = (angle >= 2u) ? 0u : (angle + 1u);
      }
    }
    else
    {
      angle = 0u;
      lv_label_set_text(lvHome.symbols.labelPitmaster, lvHome_PitSymbols[angle]);
    }
  }
  else
  {
    lv_label_set_text(lvHome.symbols.labelPitmaster, "");
  }
}

void lvHome_UpdateAlarmSymbol(boolean forceUpdate)
{
  boolean newHasAlarm = gSystem->temperatures.hasAlarm(false);
  static boolean hasAlarm = false;

  if ((hasAlarm != newHasAlarm) || forceUpdate)
  {
    if (newHasAlarm)
      lv_obj_remove_flag(lvHome.symbols.btnAlarm, LV_OBJ_FLAG_HIDDEN);
    else
      lv_obj_add_flag(lvHome.symbols.btnAlarm, LV_OBJ_FLAG_HIDDEN);
    hasAlarm = newHasAlarm;
  }
}

void lvHome_UpdateSymbols(boolean forceUpdate)
{
  WifiState newWifiState = gSystem->wlan.getWifiState();
  WifiStrength newWifiStrength = gSystem->wlan.getSignalStrength();
  const char *newWifiSymbolText = lvHome_WifiSymbolText[(uint8_t)WifiStrength::None];
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
    if (gSystem->cloud.state != 2)
      lv_obj_add_flag(lvHome.symbols.btnCloud, LV_OBJ_FLAG_HIDDEN);
    else
      lv_obj_remove_flag(lvHome.symbols.btnCloud, LV_OBJ_FLAG_HIDDEN);
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
    switch (newWifiState)
    {
    case WifiState::SoftAPNoClient:
      if (delayApSymbol) break;
      lv_label_set_text(lvHome.symbols.labelWifi, "l");
      lv_obj_remove_flag(lvHome.symbols.btnWifi, LV_OBJ_FLAG_HIDDEN);
      break;
    case WifiState::SoftAPClientConnected:
      if (delayApSymbol) break;
      lv_label_set_text(lvHome.symbols.labelWifi, "l");
      lv_obj_remove_flag(lvHome.symbols.btnWifi, LV_OBJ_FLAG_HIDDEN);
      break;
    case WifiState::ConnectedToSTA:
      lv_label_set_text(lvHome.symbols.labelWifi, newWifiSymbolText);
      lv_obj_remove_flag(lvHome.symbols.btnWifi, LV_OBJ_FLAG_HIDDEN);
      break;
    case WifiState::ConnectingToSTA:
    case WifiState::AddCredentials:
      break;
    default:
      lv_label_set_text(lvHome.symbols.labelWifi, "");
      lv_obj_add_flag(lvHome.symbols.btnWifi, LV_OBJ_FLAG_HIDDEN);
      break;
    }
    wifiState = newWifiState;
  }
}

lv_color_t htmlColorToLvColor(String htmlColor)
{
  uint32_t number = (uint32_t)strtol(htmlColor.substring(1).c_str(), NULL, 16);
  uint8_t r = number >> 16;
  uint8_t g = number >> 8 & 0xFF;
  uint8_t b = number & 0xFF;
  return lv_color_make(r, g, b);
}

void lvlvHome_UpdateSensorCb(uint8_t index, TemperatureBase *temperature, boolean settingsChanged, void *userData)
{
  lvHome_UpdateTemperature |= (true == settingsChanged) ? UPDATE_ALL : (1u << index);
}

void lvHome_TileEvent(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    lvScreen_Open(lvScreenType::Temperature, lv_obj_get_user_data((lv_obj_t *)lv_event_get_target(e)));
  }
}

void lvHome_NavigationMenuEvent(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    lvScreen_Open(lvScreenType::Display);
  }
}

void lvHome_NavigationLeftEvent(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    int8_t newPageIndex = lvHome_TempPageIndex - 1;
    uint32_t numOfTemperatures = gSystem->temperatures.getActiveCount();

    numOfTemperatures = (0u == numOfTemperatures) ? gSystem->temperatures.count() : numOfTemperatures;
    uint8_t numOfPages = (numOfTemperatures / LV_HOME_SENSORS_PER_PAGE) + 1u;

    if (newPageIndex < 0)
      newPageIndex = numOfPages - 1u;
    else if (newPageIndex >= numOfPages)
      newPageIndex = 0u;

    if (lvHome_TempPageIndex != newPageIndex)
    {
      lvHome_TempPageIndex = newPageIndex;
      lvHome_UpdateSensorTiles(true);
    }
  }
}

void lvHome_NavigationRightEvent(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    int8_t newPageIndex = lvHome_TempPageIndex + 1;
    uint32_t numOfTemperatures = gSystem->temperatures.getActiveCount();

    numOfTemperatures = (0u == numOfTemperatures) ? gSystem->temperatures.count() : numOfTemperatures;
    uint8_t numOfPages = (numOfTemperatures / LV_HOME_SENSORS_PER_PAGE) + 1u;

    if (newPageIndex < 0)
      newPageIndex = numOfPages - 1u;
    else if (newPageIndex >= numOfPages)
      newPageIndex = 0u;

    if (lvHome_TempPageIndex != newPageIndex)
    {
      lvHome_TempPageIndex = newPageIndex;
      lvHome_UpdateSensorTiles(true);
    }
  }
}

void lvHome_NavigationWifiEvent(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    lvScreen_Open(lvScreenType::Wifi);
  }
}

void lvHome_NavigationPitmasterEvent(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    lvScreen_Open(lvScreenType::Pitmaster);
  }
}
