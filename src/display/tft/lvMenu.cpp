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

static void lvMenu_HomeEvent(lv_obj_t *obj, lv_event_t event);
static void lvMenu_DisplayEvent(lv_obj_t *obj, lv_event_t event);
static void lvMenu_PitmasterEvent(lv_obj_t *obj, lv_event_t event);

void lvMenu_Create(void *userData)
{
  /* create screen for menu */
  lvMenu.screen = lv_obj_create(NULL, NULL);

  lv_obj_t *cont = lv_cont_create(lvMenu.screen, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_PRETTY_MID);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  //lv_obj_set_style_local_pad_inner(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);
  //lv_obj_set_style_local_pad_left(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 20);

  /* create temperatures button */
  lvMenu.btnHome = lv_btn_create(cont, NULL);
  lv_obj_add_protect(lvMenu.btnHome, LV_PROTECT_CLICK_FOCUS);
  lv_obj_set_style_local_value_str(lvMenu.btnHome, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "Home");
  lv_obj_set_size(lvMenu.btnHome, 100, 60);
  lv_obj_set_event_cb(lvMenu.btnHome, lvMenu_HomeEvent);

  /* create display button */
  lvMenu.btnDisplay = lv_btn_create(cont, NULL);
  lv_obj_add_protect(lvMenu.btnDisplay, LV_PROTECT_CLICK_FOCUS);
  lv_obj_set_style_local_value_str(lvMenu.btnDisplay, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "Display");
  lv_obj_set_size(lvMenu.btnDisplay, 100, 60);
  lv_obj_set_event_cb(lvMenu.btnDisplay, lvMenu_DisplayEvent);

  /* create display button */
  lvMenu.btnPitmaster = lv_btn_create(cont, NULL);
  lv_obj_add_protect(lvMenu.btnPitmaster, LV_PROTECT_CLICK_FOCUS);
  lv_obj_set_style_local_value_str(lvMenu.btnPitmaster, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, "Pitmaster");
  lv_obj_set_size(lvMenu.btnPitmaster, 100, 60);
  lv_obj_set_event_cb(lvMenu.btnPitmaster, lvMenu_PitmasterEvent);

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

void lvMenu_HomeEvent(lv_obj_t *obj, lv_event_t event)
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

void lvMenu_PitmasterEvent(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    lvScreen_Open(lvScreenType::Pitmaster);
  }
}
