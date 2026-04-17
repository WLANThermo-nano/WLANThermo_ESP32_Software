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
#include "lvDisplay.h"
#include "lvScreen.h"
#include "lv_qrcode.h"
#include "display/DisplayBase.h"
#include "display/tft/DisplayTft.h"

LV_FONT_DECLARE(Font_Gothic_A1_Medium_h16);
LV_FONT_DECLARE(Font_Nano_h24);
LV_FONT_DECLARE(Font_Roboto_Medium_h28);
LV_FONT_DECLARE(Font_Roboto_Medium_h80);

static lvDisplayType lvDisplay = {NULL};

static void lvDisplay_CreateTabBrightness(void);
static void lvDisplay_CreateTabTimeout(void);

static void lvDisplay_BtnClose(lv_obj_t *obj, lv_event_t event);
static void lvDisplay_BrightnessEvent(lv_obj_t *obj, lv_event_t event);
static void lvDisplay_TimeoutMinusEvent(lv_obj_t *obj, lv_event_t event);
static void lvDisplay_TimeoutPlusEvent(lv_obj_t *obj, lv_event_t event);
static void lvDisplay_UpdateTimeoutLabel(void);

void lvDisplay_Create(void *userData)
{
  DisplayTft *tftDisplay = (DisplayTft *)gDisplay;

  //* create screen for temperature */
  lvDisplay.screen = lv_obj_create(NULL, NULL);
  lv_obj_set_size(lvDisplay.screen, LV_HOR_RES, LV_VER_RES);

  lvDisplay.tabview = lv_tabview_create(lvDisplay.screen, NULL);
  lv_obj_set_style_local_pad_top(lvDisplay.tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_bottom(lvDisplay.tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_top(lvDisplay.tabview, LV_TABVIEW_PART_TAB_BTN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_bottom(lvDisplay.tabview, LV_TABVIEW_PART_TAB_BTN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_right(lvDisplay.tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, LV_HOR_RES / 3);
  lv_tabview_set_anim_time(lvDisplay.tabview, 0);
  lv_obj_set_style_local_text_font(lvDisplay.tabview, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Nano_h24);

  lv_obj_t *btn = lv_btn_create(lvDisplay.screen, NULL);
  lv_obj_set_event_cb(btn, lvDisplay_BtnClose);
  lv_obj_t *label = lv_label_create(btn, NULL);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);
  lv_obj_set_pos(btn, LV_DPX(335), LV_DPX(12));
  lv_obj_set_size(btn, LV_DPX(50), LV_DPX(35));

  lvDisplay_CreateTabBrightness();
  lvDisplay_CreateTabTimeout();

  lvDisplay_Update(true);

  lv_scr_load(lvDisplay.screen);
}

void lvDisplay_CreateTabBrightness(void)
{

  DisplayTft *tftDisplay = (DisplayTft *)gDisplay;
  
  /* create display tab */
  lv_obj_t *tab = lv_tabview_add_tab(lvDisplay.tabview, "t");

  /* create container */
  lv_obj_t *cont = lv_cont_create(tab, NULL);
  lv_cont_set_layout(cont, LV_LAYOUT_ROW_MID);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_left(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 60);

  /* ===== Brightness UI ===== */
  lvDisplay.sliderBrightness = lv_slider_create(cont, NULL);
  lv_obj_set_width(lvDisplay.sliderBrightness, 210);
  lv_slider_set_range(lvDisplay.sliderBrightness, 1, 10);
  lv_slider_set_value(lvDisplay.sliderBrightness, (int16_t)(tftDisplay->getUserBrightness() / 10u), LV_ANIM_OFF);
  lv_obj_set_event_cb(lvDisplay.sliderBrightness, lvDisplay_BrightnessEvent);
  lv_obj_set_style_local_value_font(lvDisplay.sliderBrightness, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, &Font_Nano_h24);
  lv_obj_set_style_local_value_str(lvDisplay.sliderBrightness, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, "t");
  lv_obj_set_style_local_value_align(lvDisplay.sliderBrightness, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_LEFT_MID);
  lv_obj_set_style_local_value_ofs_x(lvDisplay.sliderBrightness, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, -20);
}

void lvDisplay_CreateTabTimeout(void) 
{

  DisplayTft *tftDisplay = (DisplayTft *)gDisplay;

  /* create display tab */
  lv_obj_t *tab = lv_tabview_add_tab(lvDisplay.tabview, "r");

  /* create container */
  lv_obj_t *cont = lv_cont_create(tab, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_PRETTY_MID);                                         // LV_LAYOUT_ROW_MID
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);

  /* ===== Timeout UI ===== */
  
  /* Label */
  lvDisplay.labelTimeout = lv_label_create(cont, NULL);
  lv_obj_align(lvDisplay.labelTimeout, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_style_local_text_font(lvDisplay.labelTimeout, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &Font_Roboto_Medium_h28);

  /* Minus */
  lv_obj_t *btnDec = lv_btn_create(cont, NULL);
  lv_obj_set_size(btnDec, 100, 50);
  lv_obj_set_style_local_value_str(btnDec, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_MINUS);
  lv_obj_set_event_cb(btnDec, lvDisplay_TimeoutMinusEvent);

  /* Plus */
  lv_obj_t *btnInc = lv_btn_create(cont, NULL);
  lv_obj_set_size(btnInc, 100, 50);
  lv_obj_set_style_local_value_str(btnInc, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_SYMBOL_PLUS);
  lv_obj_set_event_cb(btnInc, lvDisplay_TimeoutPlusEvent);
  
  /* Initialwert aus Display holen */
  lvDisplay.timeoutIndex = tftDisplay->getTimeoutIndex();
  lvDisplay_UpdateTimeoutLabel();
}

void lvDisplay_Update(boolean forceUpdate)
{
}

void lvDisplay_Delete(void)
{
  lv_obj_del(lvDisplay.screen);
}

void lvDisplay_BtnClose(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    gDisplay->saveConfig();
    lvScreen_Open(lvScreenType::Home);
  }
}

void lvDisplay_BrightnessEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_VALUE_CHANGED == event)
  {
    DisplayTft *tftDisplay = (DisplayTft *)gDisplay;
    int16_t value = lv_slider_get_value(obj);

    tftDisplay->setUserBrightness((uint8_t)(value * 10u));
  }
}

static void lvDisplay_UpdateTimeoutLabel(void)
{
  DisplayTft *tftDisplay = (DisplayTft *)gDisplay;

  uint32_t timeout = tftDisplay->getTimeout();

  char buf[24];

  if (timeout == 0)
  {
    snprintf(buf, sizeof(buf), "Timeout: OFF");
  }
  else
  {
    snprintf(buf, sizeof(buf), "Timeout: %u s", timeout / 1000UL);
  }

  lv_label_set_text(lvDisplay.labelTimeout, buf);
}

static void lvDisplay_TimeoutMinusEvent(lv_obj_t *obj, lv_event_t event)
{
  if (event != LV_EVENT_CLICKED)
    return;

  DisplayTft *tftDisplay = (DisplayTft *)gDisplay;

  if (lvDisplay.timeoutIndex > 0)
  {
    lvDisplay.timeoutIndex--;
    tftDisplay->setTimeoutIndex(lvDisplay.timeoutIndex);
    lvDisplay_UpdateTimeoutLabel();
  }
}

static void lvDisplay_TimeoutPlusEvent(lv_obj_t *obj, lv_event_t event)
{
  if (event != LV_EVENT_CLICKED)
    return;

  DisplayTft *tftDisplay = (DisplayTft *)gDisplay;

  lvDisplay.timeoutIndex++;
  tftDisplay->setTimeoutIndex(lvDisplay.timeoutIndex);
  lvDisplay.timeoutIndex = tftDisplay->getTimeoutIndex(); // Clamp-Sicherheit
  lvDisplay_UpdateTimeoutLabel();
}