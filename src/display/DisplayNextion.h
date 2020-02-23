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
#include <ESPAsyncWebServer.h>
#include <ESPNexUpload.h>
#include "system/SystemBase.h"
#include "DisplayBase.h"

class DisplayNextion : public DisplayBase
{
public:
  DisplayNextion();
  void init();
  void update();
  void saveConfig();
  void calibrate();
  void updateFromSPIFFS();

private:
  static void setTemperatureCount();
  static void setTemperatureAllItems(TemperatureBase *temperature);
  static void setTemperatureColor(TemperatureBase *temperature);
  static void setTemperatureName(TemperatureBase *temperature);
  static void setTemperatureMin(TemperatureBase *temperature);
  static void setTemperatureMax(TemperatureBase *temperature);
  static void setTemperatureCurrent(TemperatureBase *temperature);
  static void setSymbols(boolean forceUpdate = false);
  static uint8_t getCurrentPageNumber();
  static uint32_t htmlColorToRgb565(String htmlColor);
  static void showTemperatureSettings(void *ptr);
  static void saveTemperatureSettings(void *ptr);
  static void enterSystemSettingsPage(void *ptr);
  static void saveSystemSettings(void *ptr);
  static void enterWifiSettingsPage(void *ptr);
  static void wifiNextLeft(void *ptr);
  static void wifiNextRight(void *ptr);
  static void wifiConnect(void *ptr);
  static void enterPitmasterSettingsPage(void *ptr);
  static void savePitmasterSettings(void *ptr);
  void updateWifiSettingsPage();
  void loadConfig();
  boolean initDisplay();
  static void temperatureUpdateCb(TemperatureBase *temperature, boolean settingsChanged, void *userData);
  static void task(void *parameter);
  static SystemBase *system;
  static uint32_t updateTemperature;
  static uint8_t serialTimeout;
  static boolean wifiScanInProgress;
  static ESPNexUpload nexUpload;
  static int8_t wifiIndex;
};
