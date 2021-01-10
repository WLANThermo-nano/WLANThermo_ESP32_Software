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

#include <SPI.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <rom/rtc.h>
#include "SystemBase.h"
#include "Constants.h"
#include "RecoveryMode.h"
#include "TaskConfig.h"
#include "ArduinoLog.h"

#define STRINGIFY(s) #s

#define CHECK_CYCLE(a, b) (!(a % b))
#define ONCE_PER_SECOND_CYCLE (1000 / TASK_CYCLE_TIME_SYSTEM_TASK)

char SystemBase::serialNumber[13] = "";

SystemBase::SystemBase()
{
  buzzer = NULL;
  pbGuard = NULL;
  battery = NULL;
  bluetooth = NULL;
  connect = NULL;
  sdCard = NULL;
  deviceName = "undefined";
  cpuName = "esp32";
  language = "de";
  hardwareVersion = 1u;
  pitmasterProfileCount = 0u;
  powerSaveModeSupport = false;
  powerSaveModeEnabled = false;
  damperSupport = false;
  initDone = false;
  disableTypeK = false;
  disableReceiver = false;
  wireSemaHandle = xSemaphoreCreateMutex();
  esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "NULL", &this->wirePmHandle);
}

void SystemBase::init()
{
}

void SystemBase::hwInit()
{
}

void SystemBase::run()
{
  xTaskCreatePinnedToCore(SystemBase::task, "SystemBase::task", 5000, this, TASK_PRIORITY_SYSTEM_TASK, NULL, 1);
}

void SystemBase::task(void *parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  SystemBase *system = (SystemBase *)parameter;

  for (;;)
  {
    uint32_t time = esp_timer_get_time();
    system->update();
    time = esp_timer_get_time() - time;
    //printf("Performance: %dus\n", time);

    // Wait for the next cycle.
    vTaskDelayUntil(&xLastWakeTime, TASK_CYCLE_TIME_SYSTEM_TASK);
  }
}

PitmasterProfile *SystemBase::getPitmasterProfile(uint8_t index)
{
  PitmasterProfile *prof = NULL;

  if (index < this->pitmasterProfileCount)
  {
    prof = this->profile[index];
  }

  return prof;
}

uint8_t SystemBase::getPitmasterProfileCount()
{
  return this->pitmasterProfileCount;
}

void SystemBase::update()
{
  static uint8_t cycleCounter = 0u;
  boolean buzzerAlarm = false;
  boolean enablePsm = true;

  // Increment cycle counter
  cycleCounter++;

  if ((battery != NULL) && (CHECK_CYCLE(cycleCounter, ONCE_PER_SECOND_CYCLE)))
  {
    battery->update();

    if (battery->requestsStandby())
    {
      esp_sleep_enable_timer_wakeup(10);
      esp_deep_sleep_start();
    }

    // only enable pitmaster when USB supply is connected
    if (powerSaveModeSupport)
    {
      pitmasters.enable(battery->isUsbPowered());
    }

    // disable PSM when USB supply is connected
    if(battery->isUsbPowered())
    {
      enablePsm = false;
    }
  }

  this->wireLock();
  temperatures.update();
  this->wireRelease();

  if (CHECK_CYCLE(cycleCounter, ONCE_PER_SECOND_CYCLE))
  {
    temperatures.refresh();
    pitmasters.update();

    for (uint8_t i = 0; i < temperatures.count(); i++)
    {
      if (temperatures[i] != NULL)
      {
        this->notification.check(temperatures[i]);
        AlarmStatus alarmStatus = temperatures[i]->getAlarmStatus();
        AlarmSetting alarmSetting = temperatures[i]->getAlarmSetting();
        boolean acknowledged = temperatures[i]->isAlarmAcknowledged();

        if ((alarmStatus != NoAlarm) && (false == acknowledged) && ((AlarmViaSummer == alarmSetting) || (AlarmAll == alarmSetting)))
        {
          buzzerAlarm = true;
        }
      }
    }

    if (buzzer != NULL)
    {
      if (buzzerAlarm)
      {
        buzzer->enable();
        // disable PSM when buzzer is enabled (PWM issue)
        enablePsm = false;
      }
      else
      {
        buzzer->disable();
      }

      buzzer->update();
    }

    setPowerSaveMode(enablePsm);
  }

  if(pbGuard != NULL)
  {
    pbGuard->update();
  }
}

void SystemBase::resetConfig()
{
  for (uint8_t i = 0; i < temperatures.count(); i++)
  {
    if (temperatures[i] != NULL)
    {
      temperatures[i]->loadDefaultValues(i);
    }
  }

  temperatures.setUnit(TemperatureUnit::Celsius);
  gSystem->temperatures.saveConfig();
  
  wlan.setHostName(DEFAULT_HOSTNAME + String(this->serialNumber));
  wlan.setAccessPointName(DEFAULT_APNAME);
  wlan.saveConfig();
}

void SystemBase::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();
  json["DisableTypeK"] = disableTypeK;
  json["DisableReceiver"] = disableReceiver;
  json["language"] = language;
  Settings::write(kSystem, json);
}

void SystemBase::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kSystem, &jsonBuffer);

  if (json.success())
  {
    if (json.containsKey("DisableTypeK"))
      disableTypeK = json["DisableTypeK"].as<boolean>();
    if (json.containsKey("DisableReceiver"))
      disableReceiver = json["DisableReceiver"].as<boolean>();
    if (json.containsKey("language"))
      language = json["language"].asString();
  }

  SPIFFS.begin();
  cloud.loadConfig();
  mqtt.loadConfig();
  notification.loadConfig();
  otaUpdate.loadConfig();
}

boolean SystemBase::isInitDone()
{
  return this->initDone;
}

void SystemBase::restart()
{
  WiFi.disconnect();
  delay(500);
  yield();
  RecoveryMode::zeroResetCounter();
  ESP.restart();
}

void SystemBase::wireLock()
{
  esp_pm_lock_acquire(this->wirePmHandle);
  xSemaphoreTake(this->wireSemaHandle, portMAX_DELAY);
}

void SystemBase::wireRelease()
{
  xSemaphoreGive(this->wireSemaHandle);
  esp_pm_lock_release(this->wirePmHandle);
}

String SystemBase::getDeviceName()
{
  return this->deviceName;
}

String SystemBase::getCpuName()
{
  return this->cpuName;
}

size_t SystemBase::getFlashSize()
{
  return ESP.getFlashChipSize();
}

String SystemBase::getSerialNumber()
{
  if (strlen(serialNumber) == 0u)
  {
    uint64_t chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).

    uint8_t chip[6];
    for (int i = 0; i < 6; i++)
    {
      chip[i] = uint8_t((chipid >> 8 * (5 - i)) & 0xFF);
    }

    snprintf(serialNumber, sizeof(serialNumber), "%02x%02x%02x%02x%02x%02x", chip[5], chip[4], chip[3], chip[2], chip[1], chip[0]);
  }

  return serialNumber;
}

String SystemBase::getLanguage()
{
  return this->language;
}

void SystemBase::setLanguage(String language)
{
  this->language = language;
}

uint8_t SystemBase::getHardwareVersion()
{
  return this->hardwareVersion;
}

void SystemBase::setPowerSaveMode(boolean enable)
{
  esp_pm_config_esp32_t pm_config;
  esp_err_t ret;

  if ((enable == powerSaveModeEnabled) || (false == powerSaveModeSupport))
    return;

  // only enable PSM when every caller enables it
  pm_config.light_sleep_enable = enable;
  pm_config.max_freq_mhz = 240;
  pm_config.min_freq_mhz = 240;

  if ((ret = esp_pm_configure(&pm_config)) != ESP_OK)
  {
    Serial.printf("esp_pm_configure error %s\n", ret == ESP_ERR_INVALID_ARG ? "ESP_ERR_INVALID_ARG" : "ESP_ERR_NOT_SUPPORTED");
  }
  else
  {
      Log.notice("PSM: %s" CR, (enable == true) ? "enabled" : "disabled");
      powerSaveModeEnabled = enable;
  }
}

String SystemBase::getResetReason(uint8_t cpuId)
{
  RESET_REASON reason = rtc_get_reset_reason(cpuId);
  String resetString;

  switch (reason)
  {
  case NO_MEAN:
    resetString = STRINGIFY(NO_MEAN);
    break;
  case POWERON_RESET:
    resetString = STRINGIFY(POWERON_RESET);
    break;
  case SW_RESET:
    resetString = STRINGIFY(SW_RESET);
    break;
  case OWDT_RESET:
    resetString = STRINGIFY(OWDT_RESET);
    break;
  case DEEPSLEEP_RESET:
    resetString = STRINGIFY(DEEPSLEEP_RESET);
    break;
  case SDIO_RESET:
    resetString = STRINGIFY(SDIO_RESET);
    break;
  case TG0WDT_SYS_RESET:
    resetString = STRINGIFY(TG0WDT_SYS_RESET);
    break;
  case TG1WDT_SYS_RESET:
    resetString = STRINGIFY(TG1WDT_SYS_RESET);
    break;
  case RTCWDT_SYS_RESET:
    resetString = STRINGIFY(RTCWDT_SYS_RESET);
    break;
  case INTRUSION_RESET:
    resetString = STRINGIFY(INTRUSION_RESET);
    break;
  case TGWDT_CPU_RESET:
    resetString = STRINGIFY(TGWDT_CPU_RESET);
    break;
  case SW_CPU_RESET:
    resetString = STRINGIFY(SW_CPU_RESET);
    break;
  case RTCWDT_CPU_RESET:
    resetString = STRINGIFY(RTCWDT_CPU_RESET);
    break;
  case EXT_CPU_RESET:
    resetString = STRINGIFY(EXT_CPU_RESET);
    break;
  case RTCWDT_BROWN_OUT_RESET:
    resetString = STRINGIFY(RTCWDT_BROWN_OUT_RESET);
    break;
  case RTCWDT_RTC_RESET:
    resetString = STRINGIFY(RTCWDT_RTC_RESET);
    break;
  default:
    resetString = "UNKNOWN";
  }

  return resetString;
}

boolean SystemBase::getSupportDamper()
{
  return this->damperSupport;
}