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

static void lvPitmaster_BtnClose(lv_obj_t *obj, lv_event_t event);
static void lvPitmaster_TabLimitInc(lv_obj_t *obj, lv_event_t event);
static void lvPitmaster_TabLimitDec(lv_obj_t *obj, lv_event_t event);
static void lvPitmaster_StopPitmaster(lv_obj_t *obj, lv_event_t event);

static void lvPitmaster_saveTemperature(void);
static void lvPitmaster_saveChannel(void);

void lvPitmaster_Create(void *userData)
{
  lvPitmaster_pitmaster = gSystem->pitmasters[0];
  lvPitmaster_temperature = lvPitmaster_pitmaster->getAssignedTemperature();

  /* create screen for update */
  lvPitmaster.screen = lv_obj_create(NULL, NULL);
  lv_obj_set_size(lvPitmaster.screen, LV_HOR_RES, LV_VER_RES);

  lvPitmaster.tabview = lv_tabview_create(lvPitmaster.screen, NULL);
  lv_obj_set_style_local_pad_top(lvPitmaster.tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_bottom(lvPitmaster.tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_top(lvPitmaster.tabview, LV_TABVIEW_PART_TAB_BTN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_bottom(lvPitmaster.tabview, LV_TABVIEW_PART_TAB_BTN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_right(lvPitmaster.tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, LV_HOR_RES / 3);
  lv_tabview_set_anim_time(lvPitmaster.tabview, 0);
  lv_obj_set_style_local_text_font(lvPitmaster.tabview, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Nano_h24);

  lv_obj_t *btn = lv_btn_create(lvPitmaster.screen, NULL);
  lv_obj_set_event_cb(btn, lvPitmaster_BtnClose);
  lv_obj_t *label = lv_label_create(btn, NULL);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);
  lv_obj_set_pos(btn, LV_DPX(335), LV_DPX(12));
  lv_obj_set_size(btn, LV_DPX(50), LV_DPX(35));

  lvPitmaster_CreateTarget();
  lvPitmaster_CreateChannel();
  lvPitmaster_CreateStop();

  lv_scr_load(lvPitmaster.screen);

}

void lvPitmaster_CreateTarget(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvPitmaster.tabview, "F");

  lv_obj_t *cont = lv_cont_create(tab, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_PRETTY_MID);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

  lvPitmaster.spinboxtarget = lv_spinbox_create(cont, NULL);
  lv_spinbox_set_range(lvPitmaster.spinboxtarget, -200, 9989);
  lv_spinbox_set_digit_format(lvPitmaster.spinboxtarget, 4, 3);
  lv_spinbox_set_value(lvPitmaster.spinboxtarget, ((int32_t)lvPitmaster_pitmaster->getTargetTemperature()) * 10.0f);
  lv_spinbox_step_prev(lvPitmaster.spinboxtarget);
  lv_obj_set_width(lvPitmaster.spinboxtarget, 270);
  lv_obj_align(lvPitmaster.spinboxtarget, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_text_font(lvPitmaster.spinboxtarget, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Medium_h80);

  lv_obj_t *btnDec = lv_btn_create(cont, NULL);
  lv_obj_set_size(btnDec, 100, 50);
  lv_obj_set_style_local_value_str(btnDec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_MINUS);
  lv_obj_set_user_data(btnDec, lvPitmaster.spinboxtarget);
  lv_obj_set_event_cb(btnDec, lvPitmaster_TabLimitDec);

  lv_obj_t *btnInc = lv_btn_create(cont, NULL);
  lv_obj_set_size(btnInc, 100, 50);
  lv_obj_set_style_local_value_str(btnInc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLUS);
  lv_obj_set_user_data(btnInc, lvPitmaster.spinboxtarget);
  lv_obj_set_event_cb(btnInc, lvPitmaster_TabLimitInc);

}

static void lvPitmaster_CreateChannel(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvPitmaster.tabview, "n");

  lv_obj_t *cont = lv_cont_create(tab, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_PRETTY_MID);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

  lvPitmaster.rollerType = lv_roller_create(cont, NULL);

  String channelNames;
  uint8_t channelNum = TemperatureGrp::getIndex(lvPitmaster_temperature);
  uint8_t selectedOption = 0u;

  uint8_t optionCount = 0u;

  for (uint8_t i = 0u; i < gSystem->temperatures.count(); i++)
  {
    if (optionCount > 0u)
    {
      channelNames += "\n";
    }

    channelNames += gSystem->temperatures[i]->getName();

    if (i == channelNum)
    {
      selectedOption = optionCount;
    }

    optionCount++;
  }

  lv_roller_set_options(lvPitmaster.rollerType, channelNames.c_str(), LV_ROLLER_MODE_INFINITE);
  lv_roller_set_selected(lvPitmaster.rollerType, selectedOption, LV_ANIM_OFF);

  lv_roller_set_visible_row_count(lvPitmaster.rollerType, 3u);
  lv_obj_align(lvPitmaster.rollerType, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_text_font(lvPitmaster.rollerType, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Medium_h28);

}

void lvPitmaster_CreateStop(void)
{
  lv_obj_t *tab = lv_tabview_add_tab(lvPitmaster.tabview, "0");

  lv_obj_t *cont = lv_cont_create(tab, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_PRETTY_MID);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

  lv_obj_t *btnStp = lv_btn_create(cont, NULL);
  lv_obj_set_size(btnStp, 100, 50);
  lv_obj_set_style_local_value_str(btnStp, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_STOP);
  lv_obj_set_style_local_bg_color(btnStp,LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_set_style_local_border_color(btnStp, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  lv_obj_set_pos(btnStp, LV_DPX(0), LV_DPX(100));
  lv_obj_set_event_cb(btnStp, lvPitmaster_StopPitmaster);

}

void lvPitmaster_Update(boolean forceUpdate)
{
}

void lvPitmaster_Delete(void)
{
  lv_obj_del(lvPitmaster.screen);
}

void lvPitmaster_TabLimitInc(lv_obj_t *obj, lv_event_t event)
{
  if ((LV_EVENT_SHORT_CLICKED == event) || LV_EVENT_LONG_PRESSED_REPEAT == event)
  {
    lv_spinbox_increment((lv_obj_t *)lv_obj_get_user_data(obj));
  }
}

void lvPitmaster_TabLimitDec(lv_obj_t *obj, lv_event_t event)
{
  if ((LV_EVENT_SHORT_CLICKED == event) || LV_EVENT_LONG_PRESSED_REPEAT == event)
  {
    lv_spinbox_decrement((lv_obj_t *)lv_obj_get_user_data(obj));
  }
}

void lvPitmaster_BtnClose(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvPitmaster_saveTemperature();
    lvPitmaster_saveChannel();
    gSystem->pitmasters.saveConfig();
    lvScreen_Open(lvScreenType::Home);
  }
}

void lvPitmaster_StopPitmaster(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvScreen_Open(lvScreenType::Home);
    lvPitmaster_pitmaster->setType(pm_off);
    gSystem->pitmasters.saveConfig();
    lvScreen_Open(lvScreenType::Home);
  }
}

void lvPitmaster_saveTemperature(void)
{
  float value;

  value = ((float)lv_spinbox_get_value(lvPitmaster.spinboxtarget)) / 10.0f;
  lvPitmaster_pitmaster->setTargetTemperature(value);
}

void lvPitmaster_saveChannel(void)
{
  uint32_t num;

  num = lv_roller_get_selected(lvPitmaster.rollerType);
  lvPitmaster_pitmaster->assignTemperature(gSystem->temperatures[num]);
}
