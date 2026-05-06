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
#include "display/DisplayBase.h"
#include "display/tft/DisplayTft.h"

LV_FONT_DECLARE(Font_Gothic_A1_Medium_h16);
LV_FONT_DECLARE(Font_Nano_h24);

static lvDisplayType lvDisplay = {NULL};

static void lvDisplay_BtnClose(lv_event_t *e);
static void lvDisplay_BrightnessEvent(lv_event_t *e);

void lvDisplay_Create(void *userData)
{
  DisplayTft *tftDisplay = (DisplayTft *)gDisplay;

  lvDisplay.screen = lv_obj_create(NULL);
  lv_obj_remove_flag(lvDisplay.screen, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_t *tabview = lv_tabview_create(lvDisplay.screen);
  lv_tabview_set_tab_bar_position(tabview, LV_DIR_TOP);
  lv_tabview_set_tab_bar_size(tabview, 50);
  lv_obj_t *tabBar = lv_tabview_get_tab_bar(tabview);
  lv_obj_set_width(tabBar, 50);
  lv_obj_set_style_pad_top(tabBar, 5, LV_PART_ITEMS);
  lv_obj_set_style_pad_bottom(tabBar, 5, LV_PART_ITEMS);
  lv_obj_set_style_text_font(tabBar, &Font_Nano_h24, 0);
  lv_obj_set_style_anim_duration(tabview, 0, 0);

  lv_obj_t *btn = lv_btn_create(lvDisplay.screen);
  lv_obj_add_event_cb(btn, lvDisplay_BtnClose, LV_EVENT_CLICKED, NULL);
  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);
  lv_obj_set_size(btn, 50, 35);
  lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, -5, 7);

  lv_obj_t *tab = lv_tabview_add_tab(tabview, "r");

  lv_obj_t *cont = lv_obj_create(tab);
  lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_border_width(cont, 0, 0);
  lv_obj_set_style_radius(cont, 10, 0);
  lv_obj_set_style_pad_left(cont, 60, 0);

  lv_obj_t *labelIcon = lv_label_create(cont);
  lv_obj_set_style_text_font(labelIcon, &Font_Nano_h24, 0);
  lv_label_set_text(labelIcon, "t");

  lvDisplay.sliderBrightness = lv_slider_create(cont);
  lv_obj_set_width(lvDisplay.sliderBrightness, 210);
  lv_slider_set_range(lvDisplay.sliderBrightness, 1, 10);
  lv_slider_set_value(lvDisplay.sliderBrightness, (int16_t)(tftDisplay->getBrightness() / 10u), LV_ANIM_OFF);
  lv_obj_add_event_cb(lvDisplay.sliderBrightness, lvDisplay_BrightnessEvent, LV_EVENT_VALUE_CHANGED, NULL);

  lvDisplay_Update(true);

  lv_screen_load(lvDisplay.screen);
}

void lvDisplay_Update(boolean forceUpdate)
{
}

void lvDisplay_Delete(void)
{
  lv_obj_delete(lvDisplay.screen);
}

void lvDisplay_BtnClose(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    gDisplay->saveConfig();
    lvScreen_Open(lvScreenType::Home);
  }
}

void lvDisplay_BrightnessEvent(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
  {
    DisplayTft *tftDisplay = (DisplayTft *)gDisplay;
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    int32_t value = lv_slider_get_value(slider);
    tftDisplay->setBrightness((uint8_t)(value * 10u));
  }
}
