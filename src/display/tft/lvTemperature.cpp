/***************************************************
    Copyright (C) 2021  Martin Koerner

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
#include "lvTemperature.h"
#include "lvScreen.h"
#include "temperature/TemperatureBase.h"
#include "system/SystemBase.h"

LV_FONT_DECLARE(Font_Roboto_Medium_h28);
LV_FONT_DECLARE(Font_Roboto_Medium_h80);
LV_FONT_DECLARE(Font_Nano_h24);

static lvTemperatureType lvTemperature = {NULL};
static TemperatureBase *lvTemperature_temperatureBase = NULL;
static uint32_t lvTemperature_selectedColor = 0u;

static void lvTemperature_CreateTabMin(void);
static void lvTemperature_CreateTabMax(void);
static void lvTemperature_CreateTabType(void);
static void lvTemperature_CreateTabColor(void);
static void lvTemperature_CreateTabNotif(void);

static void lvTemperature_BtnClose(lv_event_t *e);
static void lvTemperature_TabLimitInc(lv_event_t *e);
static void lvTemperature_TabLimitDec(lv_event_t *e);
static void lvTemperature_TabColorBtn(lv_event_t *e);

static void lvTemperature_saveTemperature(void);
static uint32_t lvTemperature_htmlColorStringToNum(String htmlColor);

static const uint32_t lvTemperature_colors[] = {0xFFFF00, 0xFFC002, 0x00FF00, 0xFFFFFF, 0xE46C0A, 0xC3D69B,
                                                0x0FE6F1, 0x0000FF, 0x03A923, 0xC84B32, 0xFF9B69, 0x5082BE,
                                                0xFFB1D0, 0xA6EF03, 0xD42A6B, 0xFFDA8F, 0x00B0F0, 0x948A54};

static lv_obj_t *lvTemperature_CreateSpinboxWithButtons(lv_obj_t *cont, lv_obj_t **spinboxOut,
                                                         int32_t value)
{
  *spinboxOut = lv_spinbox_create(cont);
  lv_spinbox_set_range(*spinboxOut, -200, 9989);
  lv_spinbox_set_digit_format(*spinboxOut, 4, 3);
  lv_spinbox_set_value(*spinboxOut, value);
  lv_obj_set_width(*spinboxOut, 270);
  lv_obj_align(*spinboxOut, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(*spinboxOut, &Font_Roboto_Medium_h80, 0);

  lv_obj_t *btnDec = lv_btn_create(cont);
  lv_obj_set_size(btnDec, 100, 50);
  lv_obj_set_user_data(btnDec, *spinboxOut);
  lv_obj_add_event_cb(btnDec, lvTemperature_TabLimitDec, LV_EVENT_ALL, NULL);
  lv_obj_t *labelDec = lv_label_create(btnDec);
  lv_label_set_text(labelDec, LV_SYMBOL_MINUS);
  lv_obj_center(labelDec);

  lv_obj_t *btnInc = lv_btn_create(cont);
  lv_obj_set_size(btnInc, 100, 50);
  lv_obj_set_user_data(btnInc, *spinboxOut);
  lv_obj_add_event_cb(btnInc, lvTemperature_TabLimitInc, LV_EVENT_ALL, NULL);
  lv_obj_t *labelInc = lv_label_create(btnInc);
  lv_label_set_text(labelInc, LV_SYMBOL_PLUS);
  lv_obj_center(labelInc);

  return *spinboxOut;
}

static lv_obj_t *lvTemperature_CreateTabCont(lv_obj_t *tab)
{
  lv_obj_t *cont = lv_obj_create(tab);
  lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_radius(cont, 10, 0);
  lv_obj_set_style_pad_all(cont, 4, 0);
  lv_obj_remove_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
  return cont;
}

void lvTemperature_Create(void *userData)
{
  lvTemperature_temperatureBase = (TemperatureBase *)userData;

  lvTemperature.screen = lv_obj_create(NULL);
  lv_obj_remove_flag(lvTemperature.screen, LV_OBJ_FLAG_SCROLLABLE);

  lvTemperature.tabview = lv_tabview_create(lvTemperature.screen);
  lv_tabview_set_tab_bar_position(lvTemperature.tabview, LV_DIR_TOP);
  lv_tabview_set_tab_bar_size(lvTemperature.tabview, 50);
  lv_obj_t *tabBar = lv_tabview_get_tab_bar(lvTemperature.tabview);
  lv_obj_set_width(tabBar, 320 * 2 / 3);
  lv_obj_set_style_pad_top(tabBar, 5, LV_PART_ITEMS);
  lv_obj_set_style_pad_bottom(tabBar, 5, LV_PART_ITEMS);
  lv_obj_set_style_text_font(tabBar, &Font_Nano_h24, 0);
  lv_obj_set_style_anim_duration(lvTemperature.tabview, 0, 0);

  lv_obj_t *btn = lv_btn_create(lvTemperature.screen);
  lv_obj_add_event_cb(btn, lvTemperature_BtnClose, LV_EVENT_CLICKED, NULL);
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);
  lv_obj_set_size(btn, 50, 35);
  lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -5, 7);

  lvTemperature_CreateTabMin();
  lvTemperature_CreateTabMax();
  lvTemperature_CreateTabType();
  lvTemperature_CreateTabColor();
  lvTemperature_CreateTabNotif();

  lv_screen_load(lvTemperature.screen);
}

void lvTemperature_CreateTabMin(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvTemperature.tabview, "E");
  lv_obj_t *cont = lvTemperature_CreateTabCont(tab);
  lvTemperature_CreateSpinboxWithButtons(cont, &lvTemperature.spinboxMin,
    ((int32_t)lvTemperature_temperatureBase->getMinValue()) * 10);
}

void lvTemperature_CreateTabMax(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvTemperature.tabview, "F");
  lv_obj_t *cont = lvTemperature_CreateTabCont(tab);
  lvTemperature_CreateSpinboxWithButtons(cont, &lvTemperature.spinboxMax,
    ((int32_t)lvTemperature_temperatureBase->getMaxValue()) * 10);
}

void lvTemperature_CreateTabType(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvTemperature.tabview, "n");
  lv_obj_t *cont = lvTemperature_CreateTabCont(tab);

  lvTemperature.rollerType = lv_roller_create(cont);

  String sensorTypes;
  uint8_t sensorTypeNum = lvTemperature_temperatureBase->getType();
  uint8_t selectedOption = 0u;

  if (lvTemperature_temperatureBase->isFixedSensor())
  {
    sensorTypes += "\n";
    sensorTypes = sensorTypeInfo[sensorTypeNum].name;
    sensorTypes += "\n";
    selectedOption = 1;
    lv_obj_remove_flag(lvTemperature.rollerType, LV_OBJ_FLAG_CLICKABLE);
  }
  else
  {
    uint8_t optionCount = 0u;
    for (uint8_t i = 0u; i < NUM_OF_TYPES; i++)
    {
      if (sensorTypeInfo[i].fixed != true)
      {
        if (optionCount > 0u) sensorTypes += "\n";
        sensorTypes += sensorTypeInfo[i].name;
        if (((uint8_t)sensorTypeInfo[i].type) == sensorTypeNum) selectedOption = optionCount;
        optionCount++;
      }
    }
  }

  lv_roller_set_options(lvTemperature.rollerType, sensorTypes.c_str(), LV_ROLLER_MODE_INFINITE);
  lv_roller_set_selected(lvTemperature.rollerType, selectedOption, LV_ANIM_OFF);
  lv_roller_set_visible_row_count(lvTemperature.rollerType, 3u);
  lv_obj_align(lvTemperature.rollerType, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(lvTemperature.rollerType, &Font_Roboto_Medium_h28, 0);
}

static void lvTemperature_CreateTabColor(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvTemperature.tabview, "m");

  lvTemperature_selectedColor = lvTemperature_htmlColorStringToNum(lvTemperature_temperatureBase->getColor());

  lv_obj_t *cont = lv_obj_create(tab);
  lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_radius(cont, 10, 0);
  lv_obj_set_style_pad_top(cont, 10, 0);
  lv_obj_set_style_pad_left(cont, 20, 0);

  lvTemperature.contColor = lv_obj_create(tab);
  lv_obj_set_pos(lvTemperature.contColor, lv_obj_get_x(cont), lv_obj_get_y(cont));
  lv_obj_set_height(lvTemperature.contColor, lv_obj_get_height(cont));
  lv_obj_set_width(lvTemperature.contColor, 10);
  lv_obj_set_style_bg_color(lvTemperature.contColor, lv_color_hex(lvTemperature_selectedColor), 0);
  lv_obj_set_style_border_width(lvTemperature.contColor, 0, 0);
  lv_obj_set_style_radius(lvTemperature.contColor, 10, 0);

  for (uint8_t i = 0u; i < (sizeof(lvTemperature_colors) / sizeof(uint32_t)); i++)
  {
    lv_obj_t *colorBtn = lv_btn_create(cont);
    lv_obj_set_size(colorBtn, 50, 28);
    lv_obj_set_style_bg_color(colorBtn, lv_color_hex(lvTemperature_colors[i]), 0);
    lv_obj_set_style_border_width(colorBtn, 0, 0);
    lv_obj_set_style_border_color(colorBtn, lv_color_hex(lvTemperature_colors[i]), 0);
    lv_obj_remove_flag(colorBtn, LV_OBJ_FLAG_CLICK_FOCUSABLE);
    lv_obj_add_event_cb(colorBtn, lvTemperature_TabColorBtn, LV_EVENT_CLICKED, NULL);
    lv_obj_set_user_data(colorBtn, (void *)&lvTemperature_colors[i]);
  }
}

void lvTemperature_CreateTabNotif(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvTemperature.tabview, "o");

  lv_obj_t *cont = lv_obj_create(tab);
  lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_radius(cont, 10, 0);
  lv_obj_set_style_pad_row(cont, 20, 0);
  lv_obj_set_style_pad_left(cont, 40, 0);

  lv_obj_t *rowPush = lv_obj_create(cont);
  lv_obj_remove_style_all(rowPush);
  lv_obj_set_size(rowPush, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(rowPush, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(rowPush, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_t *iconPush = lv_label_create(rowPush);
  lv_label_set_text(iconPush, "p");
  lv_obj_set_style_text_font(iconPush, &Font_Nano_h24, 0);
  lv_obj_set_style_pad_right(iconPush, 20, 0);
  lvTemperature.swPush = lv_switch_create(rowPush);

  lv_obj_t *rowBuzzer = lv_obj_create(cont);
  lv_obj_remove_style_all(rowBuzzer);
  lv_obj_set_size(rowBuzzer, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(rowBuzzer, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(rowBuzzer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_t *iconBuzzer = lv_label_create(rowBuzzer);
  lv_label_set_text(iconBuzzer, "q");
  lv_obj_set_style_text_font(iconBuzzer, &Font_Nano_h24, 0);
  lv_obj_set_style_pad_right(iconBuzzer, 20, 0);
  lvTemperature.swBuzzer = lv_switch_create(rowBuzzer);

  switch (lvTemperature_temperatureBase->getAlarmSetting())
  {
  case AlarmViaPush:
    lv_obj_add_state(lvTemperature.swPush, LV_STATE_CHECKED);
    break;
  case AlarmViaSummer:
    lv_obj_add_state(lvTemperature.swBuzzer, LV_STATE_CHECKED);
    break;
  case AlarmAll:
    lv_obj_add_state(lvTemperature.swPush, LV_STATE_CHECKED);
    lv_obj_add_state(lvTemperature.swBuzzer, LV_STATE_CHECKED);
    break;
  default:
    break;
  }
}

void lvTemperature_Update(bool forceUpdate)
{
}

void lvTemperature_Delete(void)
{
  lv_obj_delete(lvTemperature.screen);
}

void lvTemperature_BtnClose(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    lvTemperature_saveTemperature();
    lvScreen_Open(lvScreenType::Home);
  }
}

void lvTemperature_TabLimitInc(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if ((code == LV_EVENT_SHORT_CLICKED) || code == LV_EVENT_LONG_PRESSED_REPEAT)
  {
    lv_spinbox_increment((lv_obj_t *)lv_obj_get_user_data((lv_obj_t *)lv_event_get_target(e)));
  }
}

void lvTemperature_TabLimitDec(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if ((code == LV_EVENT_SHORT_CLICKED) || code == LV_EVENT_LONG_PRESSED_REPEAT)
  {
    lv_spinbox_decrement((lv_obj_t *)lv_obj_get_user_data((lv_obj_t *)lv_event_get_target(e)));
  }
}

void lvTemperature_TabColorBtn(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    lvTemperature_selectedColor = *(uint32_t *)lv_obj_get_user_data((lv_obj_t *)lv_event_get_target(e));
    lv_obj_set_style_bg_color(lvTemperature.contColor, lv_color_hex(lvTemperature_selectedColor), 0);
  }
}

void lvTemperature_saveTemperature(void)
{
  float value;

  value = ((float)lv_spinbox_get_value(lvTemperature.spinboxMin)) / 10.0f;
  lvTemperature_temperatureBase->setMinValue(value);

  value = ((float)lv_spinbox_get_value(lvTemperature.spinboxMax)) / 10.0f;
  lvTemperature_temperatureBase->setMaxValue(value);

  if (lvTemperature_temperatureBase->isFixedSensor() == false)
  {
    uint8_t optionCount = 0u;
    uint8_t selectedOption = lv_roller_get_selected(lvTemperature.rollerType);

    for (uint8_t i = 0u; i < NUM_OF_TYPES; i++)
    {
      if (sensorTypeInfo[i].fixed != true)
      {
        if (optionCount == selectedOption)
        {
          lvTemperature_temperatureBase->setType((uint8_t)sensorTypeInfo[i].type);
          break;
        }
        optionCount++;
      }
    }
  }

  lvTemperature_temperatureBase->setColor(lvTemperature_selectedColor);

  uint8_t alarmSetting = lv_obj_has_state(lvTemperature.swPush, LV_STATE_CHECKED) ? 1u : 0u;
  alarmSetting |= ((lv_obj_has_state(lvTemperature.swBuzzer, LV_STATE_CHECKED) ? 1u : 0u) << 1u);
  lvTemperature_temperatureBase->setAlarmSetting((AlarmSetting)alarmSetting);

  gSystem->temperatures.saveConfig();
}

uint32_t lvTemperature_htmlColorStringToNum(String htmlColor)
{
  return (uint32_t)strtol(htmlColor.substring(1).c_str(), NULL, 16);
}
