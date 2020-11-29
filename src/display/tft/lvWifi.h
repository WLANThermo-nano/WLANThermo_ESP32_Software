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
#pragma once

#include "Arduino.h"
#include <lvgl.h>

typedef struct lvWifi
{

  lv_obj_t *screen;
  lv_obj_t *qrCode;
  lv_obj_t *labelFirst;
  lv_obj_t *labelSecond;
  lv_obj_t *btnClose;
} lvWifiType;

void lvWifi_Create(void);
void lvWifi_Update(boolean forceUpdate);
void lvWifi_Delete(void);
