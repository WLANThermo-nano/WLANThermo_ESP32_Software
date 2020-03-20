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

#include <esp_pm.h>
#include "temperature/TemperatureGrp.h"
#include "pitmaster/PitmasterGrp.h"
#include "peripherie/Battery.h"
#include "peripherie/Buzzer.h"
#include "peripherie/SdCard.h"
#include "Notification.h"
#include "Wlan.h"
#include "Cloud.h"
#include "Mqtt.h"
#include "OtaUpdate.h"
#include "Item.h"

#define MAX_PITMASTERS 2u
#define MAX_PITMASTERPROFILES 4u

enum class SystemEdition
{
  Pro = 0,
  Lite
};

typedef struct
{
  SystemEdition edition;
  uint8_t pitmasterCount;
  uint8_t typeKCount;
  uint8_t adcChannelCount;
} SystemConfig;

class SystemBase
{
public:
  SystemBase();
  virtual void init();
  virtual void hwInit();
  PitmasterProfile *getPitmasterProfile(uint8_t index);
  Buzzer getBuzzer();
  uint8_t getPitmasterProfileCount();
  void update();
  void resetConfig();
  void saveConfig();
  void loadConfig();
  boolean isInitDone();
  Notification notification;
  Wlan wlan;
  SdCard *sdCard;
  void restart();
  void wireLock();
  void wireRelease();
  void run();

  String getDeviceName();
  String getCpuName();
  size_t getFlashSize();
  static String getSerialNumber();
  String getLanguage();
  void setLanguage(String language);
  uint8_t getHardwareVersion();
  void setPowerSaveMode(boolean enabled);
  String getResetReason(uint8_t cpuId);
  boolean getSupportDamper();
  TemperatureGrp temperatures;
  PitmasterGrp pitmasters;
  Battery *battery;
  Cloud cloud;
  Mqtt mqtt;
  OtaUpdate otaUpdate;
  Item item;

protected:
  Buzzer *buzzer;
  PitmasterProfile *profile[MAX_PITMASTERPROFILES];
  uint8_t pitmasterProfileCount;
  String deviceName;
  String cpuName;
  static char serialNumber[13];
  String language;
  uint8_t hardwareVersion;
  boolean powerSaveModeSupport;
  boolean powerSaveModeEnabled;
  boolean damperSupport;
  boolean initDone;
  SemaphoreHandle_t wireSemaHandle;
  esp_pm_lock_handle_t wirePmHandle;

private:
  static void task(void *parameter);
};

extern SystemBase* gSystem;
