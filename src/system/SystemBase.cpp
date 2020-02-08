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

#define STRINGIFY(s) #s

char SystemBase::serialNumber[13] = "";

SystemBase::SystemBase()
{
  buzzer = NULL;
  battery = NULL;
  sdCard = NULL;
  deviceName = "undefined";
  cpuName = "esp32";
  language = "de";
  hardwareVersion = 1u;
  powerSaveModeSupport = false;
  powerSaveModeEnabled = false;
  initDone = false;
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
  xTaskCreate(SystemBase::task, "SystemBase::task", 5000, this, 2, NULL);
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
    vTaskDelayUntil(&xLastWakeTime, 1000);
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
  boolean buzzerAlarm = false;

  if (battery != NULL)
  {
    battery->update();

    if (battery->requestsStandby())
    {
      esp_sleep_enable_timer_wakeup(10);
      esp_deep_sleep_start();
    }

    if (powerSaveModeSupport)
    {
      setPowerSaveMode(!battery->isUsbPowered());
      pitmasters.enable(battery->isUsbPowered());
    }
  }

  this->wireLock();
  temperatures.update();
  this->wireRelease();
  pitmasters.update();

  for (uint8_t i = 0; i < temperatures.count(); i++)
  {
    if (temperatures[i] != NULL)
    {
      this->notification.check(temperatures[i]);
      AlarmStatus alarmStatus = temperatures[i]->getAlarmStatus();
      AlarmSetting alarmSetting = temperatures[i]->getAlarmSetting();

      if ((alarmStatus != NoAlarm) && ((AlarmViaSummer == alarmSetting) || (AlarmAll == alarmSetting)))
        buzzerAlarm = true;
    }
  }

  if (buzzer != NULL)
  {
    if (buzzerAlarm)
      buzzer->enable();
    else
      buzzer->disable();

    buzzer->update();
  }
}

void SystemBase::resetConfig()
{
  for (uint8_t i = 0; i < temperatures.count(); i++)
  {
    if (temperatures[i] != NULL)
    {
      temperatures[i]->loadDefaultValues();
    }
  }

  wlan.setHostName("NANO-" + String(this->serialNumber));
  wlan.setAccessPointName("NANO-AP");
}

void SystemBase::saveConfig()
{
}

void SystemBase::loadConfig()
{
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
  delay(100);
  yield();
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

void SystemBase::setPowerSaveMode(boolean enabled)
{
  if ((enabled == powerSaveModeEnabled) || (false == powerSaveModeSupport))
    return;

  esp_pm_config_esp32_t pm_config;
  esp_err_t ret;
  pm_config.max_freq_mhz = 240;
  pm_config.min_freq_mhz = 240;
  pm_config.light_sleep_enable = enabled;

  if ((ret = esp_pm_configure(&pm_config)) != ESP_OK)
  {
    Serial.printf("esp_pm_configure error %s\n", ret == ESP_ERR_INVALID_ARG ? "ESP_ERR_INVALID_ARG" : "ESP_ERR_NOT_SUPPORTED");
  }
  else
  {
    powerSaveModeEnabled = enabled;
  }
}

String SystemBase::getResetReason(uint8_t cpuId)
{
  RESET_REASON reason = rtc_get_reset_reason(cpuId);
  String resetString;

  switch ( reason)
  {
    case NO_MEAN: resetString = STRINGIFY(NO_MEAN); break;
    case POWERON_RESET: resetString = STRINGIFY(POWERON_RESET); break;
    case SW_RESET: resetString = STRINGIFY(SW_RESET); break;
    case OWDT_RESET: resetString = STRINGIFY(OWDT_RESET); break;
    case DEEPSLEEP_RESET: resetString = STRINGIFY(DEEPSLEEP_RESET); break;
    case SDIO_RESET: resetString = STRINGIFY(SDIO_RESET); break;
    case TG0WDT_SYS_RESET: resetString = STRINGIFY(TG0WDT_SYS_RESET); break;
    case TG1WDT_SYS_RESET: resetString = STRINGIFY(TG1WDT_SYS_RESET); break;
    case RTCWDT_SYS_RESET: resetString = STRINGIFY(RTCWDT_SYS_RESET); break;
    case INTRUSION_RESET: resetString = STRINGIFY(INTRUSION_RESET); break;
    case TGWDT_CPU_RESET: resetString = STRINGIFY(TGWDT_CPU_RESET); break;
    case SW_CPU_RESET: resetString = STRINGIFY(SW_CPU_RESET); break;
    case RTCWDT_CPU_RESET: resetString = STRINGIFY(RTCWDT_CPU_RESET); break;
    case EXT_CPU_RESET: resetString = STRINGIFY(EXT_CPU_RESET); break;
    case RTCWDT_BROWN_OUT_RESET: resetString = STRINGIFY(RTCWDT_BROWN_OUT_RESET); break;
    case RTCWDT_RTC_RESET: resetString = STRINGIFY(RTCWDT_RTC_RESET); break;
    default: resetString = "UNKNOWN";
  }

  return resetString;
}