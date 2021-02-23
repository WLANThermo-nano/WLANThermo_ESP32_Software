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

static lvDisplayType lvDisplay = {NULL};

static void lvDisplay_BtnClose(lv_obj_t *obj, lv_event_t event);
static void lvDisplay_BrightnessEvent(lv_obj_t *obj, lv_event_t event);

void lvDisplay_Create(void *userData)
{
  DisplayTft *tftDisplay = (DisplayTft *)gDisplay;

  //* create screen for temperature */
  lvDisplay.screen = lv_obj_create(NULL, NULL);
  lv_obj_set_size(lvDisplay.screen, LV_HOR_RES, LV_VER_RES);

  lv_obj_t *tabview = lv_tabview_create(lvDisplay.screen, NULL);
  lv_obj_set_style_local_pad_top(tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_bottom(tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_top(tabview, LV_TABVIEW_PART_TAB_BTN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_bottom(tabview, LV_TABVIEW_PART_TAB_BTN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_right(tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, LV_HOR_RES - 50);
  lv_tabview_set_anim_time(tabview, 0);
  lv_obj_set_style_local_text_font(tabview, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Nano_h24);

  lv_obj_t *btn = lv_btn_create(lvDisplay.screen, NULL);
  lv_obj_set_event_cb(btn, lvDisplay_BtnClose);
  lv_obj_t *label = lv_label_create(btn, NULL);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);
  lv_obj_set_pos(btn, LV_DPX(335), LV_DPX(12));
  lv_obj_set_size(btn, LV_DPX(50), LV_DPX(35));

  /* create display tab */
  lv_obj_t *tab = lv_tabview_add_tab(tabview, "r");

  lv_obj_t *cont = lv_cont_create(tab, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_ROW_MID);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_left(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 60);

  lvDisplay.sliderBrightness = lv_slider_create(cont, NULL);
  lv_obj_set_width(lvDisplay.sliderBrightness, 210);
  lv_slider_set_range(lvDisplay.sliderBrightness, 1, 10);
  lv_slider_set_value(lvDisplay.sliderBrightness, (int16_t)(tftDisplay->getBrightness() / 10u), LV_ANIM_OFF);
  lv_obj_set_event_cb(lvDisplay.sliderBrightness, lvDisplay_BrightnessEvent);
  lv_obj_set_style_local_value_font(lvDisplay.sliderBrightness, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, &Font_Nano_h24);
  lv_obj_set_style_local_value_str(lvDisplay.sliderBrightness, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, "t");
  lv_obj_set_style_local_value_align(lvDisplay.sliderBrightness, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_LEFT_MID);
  lv_obj_set_style_local_value_ofs_x(lvDisplay.sliderBrightness, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, -20);

  lvDisplay_Update(true);

  lv_scr_load(lvDisplay.screen);
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

    tftDisplay->setBrightness((uint8_t)(value * 10u));
  }
}
