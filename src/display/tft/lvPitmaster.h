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
#pragma once

#include "Arduino.h"
#include <lvgl.h>

typedef struct lvPitmaster
{

  lv_obj_t *screen;
  lv_obj_t *btnClose;
  lv_obj_t *swEnable;
} lvPitmasterType;

void lvPitmaster_Create(void *userData);
void lvPitmaster_Update(boolean forceUpdate);
void lvPitmaster_Delete(void);
