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
#include "lvMenu.h"
#include "lvScreen.h"
#include "system/SystemBase.h"
#include "display/DisplayBase.h"
#include "display/tft/DisplayTft.h"

LV_FONT_DECLARE(Font_Gothic_A1_Medium_h16);

static lvMenuType lvMenu = {NULL};

static void lvMenu_TemperaturesEvent(lv_event_t *e);
static void lvMenu_DisplayEvent(lv_event_t *e);

void lvMenu_Create(void *userData)
{
  lvMenu.screen = lv_obj_create(NULL);
  lv_obj_set_style_bg_color(lvMenu.screen, lv_color_make(0x33, 0x33, 0x33), 0);
  lv_obj_remove_flag(lvMenu.screen, LV_OBJ_FLAG_SCROLLABLE);

  lvMenu.btnStyle = new lv_style_t();
  lv_style_init(lvMenu.btnStyle);
  lv_style_set_bg_color(lvMenu.btnStyle, lv_color_make(0x33, 0x33, 0x33));
  lv_style_set_border_width(lvMenu.btnStyle, 1);
  lv_style_set_radius(lvMenu.btnStyle, 0);
  lv_style_set_text_font(lvMenu.btnStyle, &Font_Gothic_A1_Medium_h16);
  lv_style_set_text_color(lvMenu.btnStyle, lv_color_white());

  lvMenu.btnTemperatures = lv_btn_create(lvMenu.screen);
  lv_obj_remove_flag(lvMenu.btnTemperatures, LV_OBJ_FLAG_CLICK_FOCUSABLE);
  lv_obj_add_style(lvMenu.btnTemperatures, lvMenu.btnStyle, 0);
  lv_obj_set_size(lvMenu.btnTemperatures, 200, 40);
  lv_obj_set_pos(lvMenu.btnTemperatures, 40, 40);
  lv_obj_add_event_cb(lvMenu.btnTemperatures, lvMenu_TemperaturesEvent, LV_EVENT_CLICKED, NULL);
  lv_obj_t *labelTemp = lv_label_create(lvMenu.btnTemperatures);
  lv_label_set_text(labelTemp, "Temperatures");
  lv_obj_center(labelTemp);

  lvMenu.btnDisplay = lv_btn_create(lvMenu.screen);
  lv_obj_remove_flag(lvMenu.btnDisplay, LV_OBJ_FLAG_CLICK_FOCUSABLE);
  lv_obj_add_style(lvMenu.btnDisplay, lvMenu.btnStyle, 0);
  lv_obj_set_size(lvMenu.btnDisplay, 200, 40);
  lv_obj_set_pos(lvMenu.btnDisplay, 40, 100);
  lv_obj_add_event_cb(lvMenu.btnDisplay, lvMenu_DisplayEvent, LV_EVENT_CLICKED, NULL);
  lv_obj_t *labelDisp = lv_label_create(lvMenu.btnDisplay);
  lv_label_set_text(labelDisp, "Display");
  lv_obj_center(labelDisp);

  lvMenu_Update(true);

  lv_screen_load(lvMenu.screen);
}

void lvMenu_Update(boolean forceUpdate)
{
}

void lvMenu_Delete(void)
{
  lv_obj_delete(lvMenu.screen);
}

void lvMenu_TemperaturesEvent(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    lvScreen_Open(lvScreenType::Home);
  }
}

void lvMenu_DisplayEvent(lv_event_t *e)
{
  if (lv_event_get_code(e) == LV_EVENT_CLICKED)
  {
    lvScreen_Open(lvScreenType::Display);
  }
}
