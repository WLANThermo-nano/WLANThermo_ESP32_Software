/*************************************************** 
    Copyright (C) 2019  Martin Koerner

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
#include "system/SystemBase.h"
#include "DisplayBase.h"

class DisplayTft : public DisplayBase
{
public:
  DisplayTft();
  void init();
  void update();
  void calibrate();

private:
  boolean initDisplay();
  static void task(void *parameter);
  static SystemBase *system;
  static uint32_t updateTemperature;
  static uint32_t updatePitmaster;
  static uint8_t serialTimeout;
  static boolean wifiScanInProgress;
  static int8_t wifiIndex;
  static uint8_t tempPageIndex;
};
