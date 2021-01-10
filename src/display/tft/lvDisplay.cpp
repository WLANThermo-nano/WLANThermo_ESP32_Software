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

static lvDisplayType lvDisplay = {NULL};

static lv_obj_t *lvDisplay_CreateSettingsLabel(lv_obj_t *parent, const char *labelText, lv_coord_t posX, lv_coord_t posY);
static void lvDisplay_CloseEvent(lv_obj_t *obj, lv_event_t event);
static void lvDisplay_BrightnessEvent(lv_obj_t *obj, lv_event_t event);

void lvDisplay_Create(void)
{
  /* create screen for display */
  lvDisplay.screen = lv_obj_create(NULL, NULL);

  lv_obj_set_style_local_bg_color(lvDisplay.screen, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));

  /* create items for brightness */
  lvDisplay.labelBrightness = lvDisplay_CreateSettingsLabel(lvDisplay.screen, "Brightness", 5, 40);

  lvDisplay.sliderBrightness = lv_slider_create(lvDisplay.screen, NULL);
  lv_obj_align(lvDisplay.sliderBrightness, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_slider_set_range(lvDisplay.sliderBrightness, 1, 10);
  lv_obj_set_size(lvDisplay.sliderBrightness, 100, 10);
  lv_obj_set_pos(lvDisplay.sliderBrightness, 5, 70);
  lv_obj_set_event_cb(lvDisplay.sliderBrightness, lvDisplay_BrightnessEvent);

  /* create close button */
  lvDisplay.btnClose = lv_btn_create(lvDisplay.screen, NULL);
  lv_obj_add_protect(lvDisplay.btnClose, LV_PROTECT_CLICK_FOCUS);
  lv_obj_set_style_local_bg_color(lvDisplay.btnClose, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_obj_set_style_local_bg_grad_color(lvDisplay.btnClose, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_obj_set_style_local_border_width(lvDisplay.btnClose, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_clip_corner(lvDisplay.btnClose, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, false);
  lv_obj_set_style_local_radius(lvDisplay.btnClose, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_size(lvDisplay.btnClose, 40, 40);
  lv_obj_set_pos(lvDisplay.btnClose, 280, 0);
  lv_obj_set_event_cb(lvDisplay.btnClose, lvDisplay_CloseEvent);

  lv_obj_t *img = lv_img_create(lvDisplay.btnClose, NULL);
  lv_obj_set_click(img, false);
  lv_img_set_src(img, LV_SYMBOL_CLOSE);
  lv_obj_set_style_local_image_recolor(img, LV_IMG_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

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

lv_obj_t *lvDisplay_CreateSettingsLabel(lv_obj_t *parent, const char *labelText, lv_coord_t posX, lv_coord_t posY)
{
  lv_obj_t *obj = lvDisplay.labelBrightness = lv_label_create(lvDisplay.screen, NULL);

  lv_label_set_text(lvDisplay.labelBrightness, labelText);
  lv_label_set_align(lvDisplay.labelBrightness, LV_LABEL_ALIGN_LEFT);
  lv_label_set_long_mode(lvDisplay.labelBrightness, LV_LABEL_LONG_BREAK);
  lv_obj_set_style_local_text_font(lvDisplay.labelBrightness, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Gothic_A1_Medium_h16);
  lv_obj_set_style_local_text_color(lvDisplay.labelBrightness, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_size(lvDisplay.labelBrightness, 100, 16);
  lv_obj_set_pos(lvDisplay.labelBrightness, posX, posY);

  return obj;
}

void lvDisplay_CloseEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvScreen_Open(lvScreenType::Menu);
  }
}

void lvDisplay_BrightnessEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_VALUE_CHANGED == event)
  {
    DisplayTft *tftDisplay = (DisplayTft *)gDisplay;
    int16_t value = lv_slider_get_value(obj);

    tftDisplay->setBrightness((uint8_t)(value * 10));
  }
}
