/***************************************************
    Copyright (C) 2020  Martin Koerner
                  2024  arnew
                  2024  Steffen Ochs

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
#include "lvPitmaster.h"
#include "lvScreen.h"
#include "system/SystemBase.h"
#include "Settings.h"

LV_FONT_DECLARE(Font_Nano_h24);
LV_FONT_DECLARE(Font_Roboto_Medium_h28);
LV_FONT_DECLARE(Font_Roboto_Medium_h80);

static Pitmaster *lvPitmaster_pitmaster = NULL;
static TemperatureBase *lvPitmaster_temperature = NULL;
static lvPitmasterType lvPitmaster = {NULL};

static void lvPitmaster_CreateTarget(void);
static void lvPitmaster_CreateChannel(void);
static void lvPitmaster_CreateStop(void);

static void lvPitmaster_BtnClose(lv_event_t *e);
static void lvPitmaster_TabLimitInc(lv_event_t *e);
static void lvPitmaster_TabLimitDec(lv_event_t *e);
static void lvPitmaster_StopPitmaster(lv_event_t *e);

static void lvPitmaster_saveTemperature(void);
static void lvPitmaster_saveChannel(void);

static lv_obj_t *lvPitmaster_CreateTabCont(lv_obj_t *tab)
{
  lv_obj_t *cont = lv_obj_create(tab);
  lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_radius(cont, 10, 0);
  return cont;
}

void lvPitmaster_Create(void *userData)
{
  lvPitmaster_pitmaster = gSystem->pitmasters[0];
  lvPitmaster_temperature = lvPitmaster_pitmaster->getAssignedTemperature();

  lvPitmaster.screen = lv_obj_create(NULL);

  lvPitmaster.tabview = lv_tabview_create(lvPitmaster.screen);
  lv_tabview_set_tab_bar_position(lvPitmaster.tabview, LV_DIR_TOP);
  lv_tabview_set_tab_bar_size(lvPitmaster.tabview, 50);
  lv_obj_t *tabBar = lv_tabview_get_tab_bar(lvPitmaster.tabview);
  lv_obj_set_width(tabBar, 320 * 2 / 3);
  lv_obj_set_style_pad_top(tabBar, 5, LV_PART_ITEMS);
  lv_obj_set_style_pad_bottom(tabBar, 5, LV_PART_ITEMS);
  lv_obj_set_style_text_font(tabBar, &Font_Nano_h24, 0);
  lv_obj_set_style_anim_duration(lvPitmaster.tabview, 0, 0);

  lv_obj_t *btn = lv_btn_create(lvPitmaster.screen);
  lv_obj_add_event_cb(btn, lvPitmaster_BtnClose, LV_EVENT_CLICKED, NULL);
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);
  lv_obj_set_pos(btn, LV_DPX(335), LV_DPX(12));
  lv_obj_set_size(btn, LV_DPX(50), LV_DPX(35));

  lvPitmaster_CreateTarget();
  lvPitmaster_CreateChannel();
  lvPitmaster_CreateStop();

  lv_screen_load(lvPitmaster.screen);
}

void lvPitmaster_CreateTarget(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvPitmaster.tabview, "F");
  lv_obj_t *cont = lvPitmaster_CreateTabCont(tab);

  lvPitmaster.spinboxtarget = lv_spinbox_create(cont);
  lv_spinbox_set_range(lvPitmaster.spinboxtarget, -200, 9989);
  lv_spinbox_set_digit_format(lvPitmaster.spinboxtarget, 4, 3);
  lv_spinbox_set_value(lvPitmaster.spinboxtarget, ((int32_t)lvPitmaster_pitmaster->getTargetTemperature()) * 10);
  lv_obj_set_width(lvPitmaster.spinboxtarget, 270);
  lv_obj_align(lvPitmaster.spinboxtarget, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(lvPitmaster.spinboxtarget, &Font_Roboto_Medium_h80, 0);

  lv_obj_t *btnDec = lv_btn_create(cont);
  lv_obj_set_size(btnDec, 100, 50);
  lv_obj_set_user_data(btnDec, lvPitmaster.spinboxtarget);
  lv_obj_add_event_cb(btnDec, lvPitmaster_TabLimitDec, LV_EVENT_ALL, NULL);
  lv_obj_t *labelDec = lv_label_create(btnDec);
  lv_label_set_text(labelDec, LV_SYMBOL_MINUS);
  lv_obj_center(labelDec);

  lv_obj_t *btnInc = lv_btn_create(cont);
  lv_obj_set_size(btnInc, 100, 50);
  lv_obj_set_user_data(btnInc, lvPitmaster.spinboxtarget);
  lv_obj_add_event_cb(btnInc, lvPitmaster_TabLimitInc, LV_EVENT_ALL, NULL);
  lv_obj_t *labelInc = lv_label_create(btnInc);
  lv_label_set_text(labelInc, LV_SYMBOL_PLUS);
  lv_obj_center(labelInc);
}

static void lvPitmaster_CreateChannel(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvPitmaster.tabview, "n");
  lv_obj_t *cont = lvPitmaster_CreateTabCont(tab);

  lvPitmaster.rollerType = lv_roller_create(cont);

  String channelNames;
  uint8_t channelNum = TemperatureGrp::getIndex(lvPitmaster_temperature);
  uint8_t selectedOption = 0u;
  uint8_t optionCount = 0u;

  for (uint8_t i = 0u; i < gSystem->temperatures.count(); i++)
  {
    if (optionCount > 0u) channelNames += "\n";
    channelNames += gSystem->temperatures[i]->getName();
    if (i == channelNum) selectedOption = optionCount;
    optionCount++;
  }

  lv_roller_set_options(lvPitmaster.rollerType, channelNames.c_str(), LV_ROLLER_MODE_INFINITE);
  lv_roller_set_selected(lvPitmaster.rollerType, selectedOption, LV_ANIM_OFF);
  lv_roller_set_visible_row_count(lvPitmaster.rollerType, 3u);
  lv_obj_align(lvPitmaster.rollerType, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_text_font(lvPitmaster.rollerType, &Font_Roboto_Medium_h28, 0);
}

void lvPitmaster_CreateStop(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvPitmaster.tabview, "0");
  lv_obj_t *cont = lvPitmaster_CreateTabCont(tab);

  lv_obj_t *btnStp = lv_btn_create(cont);
  lv_obj_set_size(btnStp, 100, 50);
  lv_obj_set_style_bg_color(btnStp, lv_color_make(0xFF, 0x00, 0x00), 0);
  lv_obj_set_style_border_color(btnStp, lv_color_make(0xFF, 0x00, 0x00), 0);
  lv_obj_add_event_cb(btnStp, lvPitmaster_StopPitmaster, LV_EVENT_CLICKED, NULL);
  lv_obj_t *labelStp = lv_label_create(btnStp);
  lv_label_set_text(labelStp, LV_SYMBOL_STOP);
  lv_obj_center(labelStp);
}

void lvPitmaster_Update(boolean forceUpdate)
{
}

void lvPitmaster_Delete(void)
{
  lv_obj_delete(lvPitmaster.screen);
}

void lvPitmaster_TabLimitInc(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if ((code == LV_EVENT_SHORT_CLICKED) || code == LV_EVENT_LONG_PRESSED_REPEAT)
  {
    lv_spinbox_increment((lv_obj_t *)lv_obj_get_user_data((lv_obj_t *)lv_event_get_target(e)));
  }
}

void lvPitmaster_TabLimitDec(lv_event_t *e)
{
  lv_event_code_t code = lv_event_get_code(e);
  if ((code == LV_EVENT_SHORT_CLICKED) || code == LV_EVENT_LONG_PRESSED_REPEAT)
  {
    lv_spinbox_decrement((lv_obj_t *)lv_obj_get_user_data((lv_obj_t *)lv_event_get_target(e)));
  }
}

void lvPitmaster_BtnClose(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    lvPitmaster_saveTemperature();
    lvPitmaster_saveChannel();
    gSystem->pitmasters.saveConfig();
    lvScreen_Open(lvScreenType::Home);
  }
}

void lvPitmaster_StopPitmaster(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    lvPitmaster_pitmaster->setType(pm_off);
    gSystem->pitmasters.saveConfig();
    lvScreen_Open(lvScreenType::Home);
  }
}

void lvPitmaster_saveTemperature(void)
{
  float value = ((float)lv_spinbox_get_value(lvPitmaster.spinboxtarget)) / 10.0f;
  lvPitmaster_pitmaster->setTargetTemperature(value);
}

void lvPitmaster_saveChannel(void)
{
  uint32_t num = lv_roller_get_selected(lvPitmaster.rollerType);
  lvPitmaster_pitmaster->assignTemperature(gSystem->temperatures[num]);
}
