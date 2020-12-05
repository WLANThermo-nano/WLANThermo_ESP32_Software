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
#include "system\SystemBase.h"
#include "display\DisplayBase.h"
#include "display\tft\DisplayTft.h"

LV_FONT_DECLARE(Font_Gothic_A1_Medium_h16);

static lvMenuType lvMenu = {NULL};

static void lvMenu_TemperaturesEvent(lv_obj_t *obj, lv_event_t event);
static void lvMenu_DisplayEvent(lv_obj_t *obj, lv_event_t event);

void lvMenu_Create(void)
{
  /* create screen for menu */
  lvMenu.screen = lv_obj_create(NULL, NULL);
  lv_obj_set_style_local_bg_color(lvMenu.screen, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));

  /* create style for buttons */
  lvMenu.btnStyle = new lv_style_t();
  lv_style_init(lvMenu.btnStyle);
  lv_style_set_bg_color(lvMenu.btnStyle, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_style_set_bg_grad_color(lvMenu.btnStyle, LV_STATE_DEFAULT, LV_COLOR_MAKE(0x33, 0x33, 0x33));
  lv_style_set_border_width(lvMenu.btnStyle, LV_STATE_DEFAULT, 1);
  lv_style_set_clip_corner(lvMenu.btnStyle, LV_STATE_DEFAULT, false);
  lv_style_set_radius(lvMenu.btnStyle, LV_STATE_DEFAULT, 0);
  lv_style_set_value_font(lvMenu.btnStyle, LV_STATE_DEFAULT, &Font_Gothic_A1_Medium_h16);
  lv_style_set_value_color(lvMenu.btnStyle, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_style_set_value_align(lvMenu.btnStyle, LV_STATE_DEFAULT, LV_ALIGN_CENTER);

  /* create temperatures button */
  lvMenu.btnTemperatures = lv_btn_create(lvMenu.screen, NULL);
  lv_obj_add_protect(lvMenu.btnTemperatures, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvMenu.btnTemperatures, LV_CONT_PART_MAIN, lvMenu.btnStyle);
  lv_obj_set_style_local_value_str(lvMenu.btnTemperatures, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "Temperatures");
  lv_obj_set_size(lvMenu.btnTemperatures, 200, 40);
  lv_obj_set_pos(lvMenu.btnTemperatures, 40, 40);
  lv_obj_set_event_cb(lvMenu.btnTemperatures, lvMenu_TemperaturesEvent);

  /* create display button */
  lvMenu.btnDisplay = lv_btn_create(lvMenu.screen, NULL);
  lv_obj_add_protect(lvMenu.btnDisplay, LV_PROTECT_CLICK_FOCUS);
  lv_obj_add_style(lvMenu.btnDisplay, LV_CONT_PART_MAIN, lvMenu.btnStyle);
  lv_obj_set_style_local_value_str(lvMenu.btnDisplay, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "Display");
  lv_obj_set_size(lvMenu.btnDisplay, 200, 40);
  lv_obj_set_pos(lvMenu.btnDisplay, 40, 100);
  lv_obj_set_event_cb(lvMenu.btnDisplay, lvMenu_DisplayEvent);

  lvMenu_Update(true);

  lv_scr_load(lvMenu.screen);
}

void lvMenu_Update(boolean forceUpdate)
{
  
}

void lvMenu_Delete(void)
{
  lv_obj_del(lvMenu.screen);
}

void lvMenu_TemperaturesEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvScreen_Open(lvScreenType::Home);
  }
}

void lvMenu_DisplayEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvScreen_Open(lvScreenType::Display);
  }
}
