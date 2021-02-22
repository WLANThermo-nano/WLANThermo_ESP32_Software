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

static void lvTemperature_BtnClose(lv_obj_t *obj, lv_event_t event);
static void lvTemperature_TabLimitInc(lv_obj_t *obj, lv_event_t event);
static void lvTemperature_TabLimitDec(lv_obj_t *obj, lv_event_t event);
static void lvTemperature_TabColorBtn(lv_obj_t *obj, lv_event_t event);

static void lvTemperature_saveTemperature(void);
static uint32_t lvTemperature_htmlColorStringToNum(String htmlColor);

static const uint32_t lvTemperature_colors[] = {0xFFFF00, 0xFFC002, 0x00FF00, 0xFFFFFF, 0xE46C0A, 0xC3D69B,
                                                0x0FE6F1, 0x0000FF, 0x03A923, 0xC84B32, 0xFF9B69, 0x5082BE,
                                                0xFFB1D0, 0xA6EF03, 0xD42A6B, 0xFFDA8F, 0x00B0F0, 0x948A54};

void lvTemperature_Create(void *userData)
{
  lvTemperature_temperatureBase = (TemperatureBase *)userData;

  /* create screen for temperature */
  lvTemperature.screen = lv_obj_create(NULL, NULL);
  lv_obj_set_size(lvTemperature.screen, LV_HOR_RES, LV_VER_RES);

  lvTemperature.tabview = lv_tabview_create(lvTemperature.screen, NULL);
  lv_obj_set_style_local_pad_top(lvTemperature.tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_bottom(lvTemperature.tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_top(lvTemperature.tabview, LV_TABVIEW_PART_TAB_BTN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_bottom(lvTemperature.tabview, LV_TABVIEW_PART_TAB_BTN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_right(lvTemperature.tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, LV_HOR_RES / 3);
  lv_tabview_set_anim_time(lvTemperature.tabview, 0);
  lv_obj_set_style_local_text_font(lvTemperature.tabview, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Nano_h24);

  lv_obj_t *btn = lv_btn_create(lvTemperature.screen, NULL);
  lv_obj_set_event_cb(btn, lvTemperature_BtnClose);
  lv_obj_t *label = lv_label_create(btn, NULL);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);
  lv_obj_set_pos(btn, LV_DPX(335), LV_DPX(12));
  lv_obj_set_size(btn, LV_DPX(50), LV_DPX(35));

  lvTemperature_CreateTabMin();
  lvTemperature_CreateTabMax();
  lvTemperature_CreateTabType();
  lvTemperature_CreateTabColor();

  lv_scr_load(lvTemperature.screen);
}

void lvTemperature_CreateTabMin(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvTemperature.tabview, "E");

  lv_obj_t *cont = lv_cont_create(tab, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_PRETTY_MID);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

  lvTemperature.spinboxMin = lv_spinbox_create(cont, NULL);
  lv_spinbox_set_range(lvTemperature.spinboxMin, -200, 9989);
  lv_spinbox_set_digit_format(lvTemperature.spinboxMin, 4, 3);
  lv_spinbox_set_value(lvTemperature.spinboxMin, ((int32_t)lvTemperature_temperatureBase->getMinValue()) * 10.0f);
  lv_spinbox_step_prev(lvTemperature.spinboxMin);
  lv_obj_set_width(lvTemperature.spinboxMin, 270);
  lv_obj_align(lvTemperature.spinboxMin, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_text_font(lvTemperature.spinboxMin, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Medium_h80);

  lv_obj_t *btnDec = lv_btn_create(cont, NULL);
  lv_obj_set_size(btnDec, 100, 50);
  lv_obj_set_style_local_value_str(btnDec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_MINUS);
  lv_obj_set_user_data(btnDec, lvTemperature.spinboxMin);
  lv_obj_set_event_cb(btnDec, lvTemperature_TabLimitDec);

  lv_obj_t *btnInc = lv_btn_create(cont, NULL);
  lv_obj_set_size(btnInc, 100, 50);
  lv_obj_set_style_local_value_str(btnInc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLUS);
  lv_obj_set_user_data(btnInc, lvTemperature.spinboxMin);
  lv_obj_set_event_cb(btnInc, lvTemperature_TabLimitInc);
}

void lvTemperature_CreateTabMax(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvTemperature.tabview, "F");

  lv_obj_t *cont = lv_cont_create(tab, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_PRETTY_MID);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

  lvTemperature.spinboxMax = lv_spinbox_create(cont, NULL);
  lv_spinbox_set_range(lvTemperature.spinboxMax, -200, 9989);
  lv_spinbox_set_digit_format(lvTemperature.spinboxMax, 4, 3);
  lv_spinbox_set_value(lvTemperature.spinboxMax, ((int32_t)lvTemperature_temperatureBase->getMaxValue()) * 10.0f);
  lv_spinbox_step_prev(lvTemperature.spinboxMax);
  lv_obj_set_width(lvTemperature.spinboxMax, 270);
  lv_obj_align(lvTemperature.spinboxMax, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_text_font(lvTemperature.spinboxMax, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Medium_h80);

  lv_obj_t *btnDec = lv_btn_create(cont, NULL);
  lv_obj_set_size(btnDec, 100, 50);
  lv_obj_set_style_local_value_str(btnDec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_MINUS);
  lv_obj_set_user_data(btnDec, lvTemperature.spinboxMax);
  lv_obj_set_event_cb(btnDec, lvTemperature_TabLimitDec);

  lv_obj_t *btnInc = lv_btn_create(cont, NULL);
  lv_obj_set_size(btnInc, 100, 50);
  lv_obj_set_style_local_value_str(btnInc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLUS);
  lv_obj_set_user_data(btnInc, lvTemperature.spinboxMax);
  lv_obj_set_event_cb(btnInc, lvTemperature_TabLimitInc);
}

void lvTemperature_CreateTabType(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvTemperature.tabview, "n");

  lv_obj_t *cont = lv_cont_create(tab, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_PRETTY_MID);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

  lvTemperature.rollerType = lv_roller_create(cont, NULL);

  String sensorTypes;
  uint8_t sensorTypeNum = lvTemperature_temperatureBase->getType();
  uint8_t selectedOption = 0u;

  if (lvTemperature_temperatureBase->isFixedSensor())
  {
    /* Add dummy elements for fixed sensors and disable click */
    sensorTypes += "\n";
    sensorTypes = sensorTypeInfo[sensorTypeNum].name;
    sensorTypes += "\n";
    selectedOption = 1;
    lv_obj_set_click(lvTemperature.rollerType, false);
  }
  else
  {
    uint8_t optionCount = 0u;

    for (uint8_t i = 0u; i < NUM_OF_TYPES; i++)
    {
      if (sensorTypeInfo[i].fixed != true)
      {
        if (optionCount > 0u)
        {
          sensorTypes += "\n";
        }

        sensorTypes += sensorTypeInfo[i].name;

        if (((uint8_t)sensorTypeInfo[i].type) == sensorTypeNum)
        {
          selectedOption = optionCount;
        }

        optionCount++;
      }
    }
  }

  lv_roller_set_options(lvTemperature.rollerType, sensorTypes.c_str(), LV_ROLLER_MODE_INFINITE);
  lv_roller_set_selected(lvTemperature.rollerType, selectedOption, LV_ANIM_OFF);

  lv_roller_set_visible_row_count(lvTemperature.rollerType, 3u);
  lv_obj_align(lvTemperature.rollerType, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_text_font(lvTemperature.rollerType, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Medium_h28);
}

static void lvTemperature_CreateTabColor(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvTemperature.tabview, "m");

  lvTemperature_selectedColor = lvTemperature_htmlColorStringToNum(lvTemperature_temperatureBase->getColor());

  lv_obj_t *cont = lv_cont_create(tab, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_PRETTY_MID);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_top(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_left(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);

  lvTemperature.contColor = lv_cont_create(tab, NULL);
  lv_obj_set_pos(lvTemperature.contColor, lv_obj_get_x(cont), lv_obj_get_y(cont));
  lv_obj_set_height(lvTemperature.contColor, lv_obj_get_height(cont));
  lv_obj_set_width(lvTemperature.contColor, 10);
  lv_obj_set_style_local_bg_color(lvTemperature.contColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(lvTemperature_selectedColor));
  lv_obj_set_style_local_border_width(lvTemperature.contColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(lvTemperature.contColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

  for (uint8_t i = 0u; i < (sizeof(lvTemperature_colors) / sizeof(uint32_t)); i++)
  {
    lv_obj_t *btn = lv_btn_create(cont, NULL);
    lv_obj_set_size(btn, 50, 28);
    lv_obj_set_style_local_bg_color(btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(lvTemperature_colors[i]));
    lv_obj_set_style_local_border_width(btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
    lv_obj_set_style_local_border_color(btn, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(lvTemperature_colors[i]));
    lv_obj_add_protect(btn, LV_PROTECT_CLICK_FOCUS);
    lv_obj_set_event_cb(btn, lvTemperature_TabColorBtn);
    lv_obj_set_user_data(btn, (void *)&lvTemperature_colors[i]);
  }
}

void lvTemperature_CreateTabNotif(void)
{

  lv_obj_t *tab = lv_tabview_add_tab(lvTemperature.tabview, "o");
}

void lvTemperature_Update(bool forceUpdate)
{
}

void lvTemperature_Delete(void)
{
  lv_obj_del(lvTemperature.screen);
}

void lvTemperature_BtnClose(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvTemperature_saveTemperature();
    lvScreen_Open(lvScreenType::Home);
  }
}

void lvTemperature_TabLimitInc(lv_obj_t *obj, lv_event_t event)
{
  if ((LV_EVENT_SHORT_CLICKED == event) || LV_EVENT_LONG_PRESSED_REPEAT == event)
  {
    lv_spinbox_increment((lv_obj_t *)lv_obj_get_user_data(obj));
  }
}

void lvTemperature_TabLimitDec(lv_obj_t *obj, lv_event_t event)
{
  if ((LV_EVENT_SHORT_CLICKED == event) || LV_EVENT_LONG_PRESSED_REPEAT == event)
  {
    lv_spinbox_decrement((lv_obj_t *)lv_obj_get_user_data(obj));
  }
}

void lvTemperature_TabColorBtn(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvTemperature_selectedColor = *(uint32_t *)lv_obj_get_user_data(obj);
    lv_obj_set_style_local_bg_color(lvTemperature.contColor, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(lvTemperature_selectedColor));
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

  gSystem->temperatures.saveConfig();
}

uint32_t lvTemperature_htmlColorStringToNum(String htmlColor)
{
  // Get rid of '#' and convert it to integer
  return (uint32_t)strtol(htmlColor.substring(1).c_str(), NULL, 16);
}
