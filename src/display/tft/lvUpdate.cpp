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
#include "lvUpdate.h"
#include "lvScreen.h"
#include "system/SystemBase.h"

LV_FONT_DECLARE(Font_Nano_h24);

extern const uint16_t DisplayTftStartScreenImg[25400];

static lvUpdateType lvUpdate = {NULL};

void lvUpdate_Create(void *userData)
{
  /* create screen for update */
  lvUpdate.screen = lv_obj_create(NULL, NULL);

  lv_obj_t *cont = lv_cont_create(lvUpdate.screen, NULL);
  lv_cont_set_fit(cont, LV_FIT_PARENT);
  lv_cont_set_layout(cont, LV_LAYOUT_ROW_MID);
  lv_obj_set_style_local_pad_left(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 40);
  lv_obj_set_style_local_border_width(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_radius(cont, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lv_obj_t *label = lv_label_create(cont, NULL);
  lv_label_set_text(label, "X");
  lv_obj_set_style_local_text_font(label, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, &Font_Nano_h24);

  lvUpdate.bar = lv_bar_create(cont, NULL);
  lv_obj_set_size(lvUpdate.bar, 200, 25);
  lv_obj_align(lvUpdate.bar, NULL, LV_ALIGN_CENTER, 0, 0);

  lvUpdate_Update(true);

  lv_scr_load(lvUpdate.screen);
}

void lvUpdate_Update(boolean forceUpdate)
{
  lv_bar_set_value(lvUpdate.bar, gSystem->otaUpdate.getUpdateProgress(), LV_ANIM_OFF);
}

void lvUpdate_Delete(void)
{
  lv_obj_del(lvUpdate.screen);
}
