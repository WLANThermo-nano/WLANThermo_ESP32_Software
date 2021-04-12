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
#include "lvPitmaster.h"
#include "lvScreen.h"
#include "lv_qrcode.h"
#include "system/SystemBase.h"

LV_FONT_DECLARE(Font_Gothic_A1_Medium_h16);
LV_FONT_DECLARE(Font_Nano_h24);

static lvPitmasterType lvPitmaster = {NULL};

static void lvPitmaster_BtnClose(lv_obj_t *obj, lv_event_t event);
static void lvPitmaster_BrightnessEvent(lv_obj_t *obj, lv_event_t event);

void lvPitmaster_Create(void *userData)
{
  //* create screen for pitmaster */
  lvPitmaster.screen = lv_obj_create(NULL, NULL);
  lv_obj_set_size(lvPitmaster.screen, LV_HOR_RES, LV_VER_RES);

  lv_obj_t *tabview = lv_tabview_create(lvPitmaster.screen, NULL);
  lv_obj_set_style_local_pad_top(tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_bottom(tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_top(tabview, LV_TABVIEW_PART_TAB_BTN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_bottom(tabview, LV_TABVIEW_PART_TAB_BTN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_pad_right(tabview, LV_TABVIEW_PART_TAB_BG, LV_STATE_DEFAULT, LV_HOR_RES - 50);
  lv_tabview_set_anim_time(tabview, 0);
  lv_obj_set_style_local_text_font(tabview, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Nano_h24);

  lv_obj_t *btn = lv_btn_create(lvPitmaster.screen, NULL);
  lv_obj_set_event_cb(btn, lvPitmaster_BtnClose);
  lv_obj_t *label = lv_label_create(btn, NULL);
  lv_label_set_text(label, LV_SYMBOL_CLOSE);
  lv_obj_set_pos(btn, LV_DPX(335), LV_DPX(12));
  lv_obj_set_size(btn, LV_DPX(50), LV_DPX(35));

  /* create pitmaster on/off tab */
  lv_obj_t *tab = lv_tabview_add_tab(tabview, "r");

  lv_obj_t *cont = lv_cont_create(tab, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_ROW_MID);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_left(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 60);

  lvPitmaster.swEnable = lv_switch_create(cont, NULL);
  lv_obj_set_style_local_value_font(lvPitmaster.swEnable, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, &Font_Nano_h24);
  lv_obj_set_style_local_value_str(lvPitmaster.swEnable, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, "p");
  lv_obj_set_style_local_value_align(lvPitmaster.swEnable, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_LEFT_MID);
  lv_obj_set_style_local_value_ofs_x(lvPitmaster.swEnable, LV_SWITCH_PART_BG, LV_STATE_DEFAULT, -20);

  Pitmaster *pm = gSystem->pitmasters[0];

  if(pm != NULL)
  {
    if(pm->getType() != pm_off)
    {
      lv_switch_on(lvPitmaster.swEnable, LV_ANIM_OFF);
    }
  }

  lvPitmaster_Update(true);

  lv_scr_load(lvPitmaster.screen);
}

void lvPitmaster_Update(boolean forceUpdate)
{
}

void lvPitmaster_Delete(void)
{
  lv_obj_del(lvPitmaster.screen);
}

void lvPitmaster_BtnClose(lv_obj_t *obj, lv_event_t event)
{
  if (LV_EVENT_CLICKED == event)
  {
    Pitmaster *pm = gSystem->pitmasters[0];

    if(pm != NULL)
    {
      PitmasterType newType = (lv_switch_get_state(lvPitmaster.swEnable)) ? pm->getTypeLast() : pm_off;
      pm->setType(newType);
      gSystem->pitmasters.saveConfig();
    }

    lvScreen_Open(lvScreenType::Home);
  }
}
