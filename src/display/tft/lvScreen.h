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

typedef void (*lvMenu_Create_FuncPtr)(void *userData);
typedef void (*lvMenu_Update_FuncPtr)(boolean forceUpdate);
typedef void (*lvMenu_Delete_FuncPtr)(void);

typedef enum class lvScreen
{
  None = 0,
  Menu,
  Home,
  Wifi,
  Display,
  Temperature,
  Max
} lvScreenType;

typedef struct
{
  lvScreenType screen;
  lvMenu_Create_FuncPtr createFunc;
  lvMenu_Update_FuncPtr updateFunc;
  lvMenu_Delete_FuncPtr deleteFunc;

} lvScreenFuncType;

void lvScreen_Open(lvScreenType screen, void *userData = NULL);
void lvScreen_Update(void);
