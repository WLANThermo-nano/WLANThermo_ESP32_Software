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
#include "DisplayNextion.h"
#include "Settings.h"
#include "Nextion.h"
#include "SPIFFS.h"
#include "TaskConfig.h"

extern "C"
{
#include <rom/miniz.h>
}

#define OUTBUFFER_SIZE (32u * 1024u)
#define INBUFFER_SIZE (32u * 1024u)
#define NEXTION_SPIFFS_UPDATE_FILENAME "/nextion.tft.zlib"

#if defined HW_MINI_V1
#define NEXTION_DIRECTION_STRING "-180"
#define NEXTION_DIRECTION_ENUM DisplayOrientation::_180
#else
#define NEXTION_DIRECTION_STRING "-0"
#define NEXTION_DIRECTION_ENUM DisplayOrientation::_0
#endif

#define NEXTION_TEMPERATURES_PER_PAGE 6u
#define UPDATE_ALL 0xFFFFFFFFu
#define NEXTION_SERIAL_TIMEOUT 100u
#define NEXTION_INVALID_PAGE 0xFFu
#define NEXTION_RETURN_CURRENT_PAGE_ID 0x66u

#define QUERY(a)      \
  serialTimeout = 5u; \
  while (((a) != true) && serialTimeout-- > 0u)

#define DONT_CARE 0u
#define NEXTION_COLOR_NO_ALARM 0xFFFFu
#define NEXTION_COLOR_MIN_ALARM 0x4D3Fu
#define NEXTION_COLOR_MAX_ALARM 0xF800u

uint16_t AlarmColorMap[3u] = {NEXTION_COLOR_NO_ALARM, NEXTION_COLOR_MIN_ALARM, NEXTION_COLOR_MAX_ALARM};

/* PAGE ID */
#define PAGE_BOOT_ID 0u
#define PAGE_TEMP_MAIN_ID 1u
#define PAGE_TEMP_LOAD_ID 2u
#define PAGE_TEMP_SETTINGS_ID 3u
#define PAGE_MENU_ID 5u
#define PAGE_WIFI_ID 6u
#define PAGE_SYSTEM_SETTINGS_ID 8u
#define PAGE_PITMASTER_SETTINGS_ID 9u
#define PAGE_PITM_MENU_ID 10

#define HOTSPOT_TEMP_SETTINGS_SAVE 26u
#define HOTSPOT_SYSTEM_SETTINGS_SAVE 9u
#define HOTSPOT_PITMASTER_SETTINGS_SAVE 28u
#define BUTTON_CHANNEL_ID 10u
#define BUTTON_TEMPB_ID 64u
#define BUTTON_TEMPF_ID 65u
#define BUTTON_ALARM_ID 73u
#define HOTSPOT_TEMP0_ID 66u
#define HOTSPOT_TEMP1_ID 67u
#define HOTSPOT_TEMP2_ID 68u
#define HOTSPOT_TEMP3_ID 69u
#define HOTSPOT_TEMP4_ID 70u
#define HOTSPOT_TEMP5_ID 71u
#define BUTTON_MENU_WLAN_ID 2u
#define BUTTON_MENU_PITMASTER1_ID 4u
#define BUTTON_MENU_PITMASTER2_ID 5u
#define BUTTON_MENU_SYSTEM_ID 4u
#define TEXT_WIFI_CONNECT_ID 1u
#define BUTTON_WIFI_BUTTON_LEFT_ID 10u
#define BUTTON_WIFI_BUTTON_RIGHT_ID 11u

#define HOTSPOT_TEMP_SETTINGS_SAVE 26u
#define HOTSPOT_SYSTEM_SETTINGS_SAVE 9u
#define HOTSPOT_PITMASTER_SETTINGS_SAVE 28u

NexHotspot nexTemperatures[NEXTION_TEMPERATURES_PER_PAGE] = {
    NexHotspot(PAGE_TEMP_MAIN_ID, HOTSPOT_TEMP0_ID, ""),
    NexHotspot(PAGE_TEMP_MAIN_ID, HOTSPOT_TEMP1_ID, ""),
    NexHotspot(PAGE_TEMP_MAIN_ID, HOTSPOT_TEMP2_ID, ""),
    NexHotspot(PAGE_TEMP_MAIN_ID, HOTSPOT_TEMP3_ID, ""),
    NexHotspot(PAGE_TEMP_MAIN_ID, HOTSPOT_TEMP4_ID, ""),
    NexHotspot(PAGE_TEMP_MAIN_ID, HOTSPOT_TEMP5_ID, "")};

static NexButton tempButtonB = NexButton(PAGE_TEMP_MAIN_ID, BUTTON_TEMPB_ID, "");
static NexButton tempButtonF = NexButton(PAGE_TEMP_MAIN_ID, BUTTON_TEMPF_ID, "");
static NexButton alarmButton = NexButton(PAGE_TEMP_MAIN_ID, BUTTON_ALARM_ID, "");

static NexButton pitButtonChannel = NexButton(PAGE_PITMASTER_SETTINGS_ID, BUTTON_CHANNEL_ID, "");

NexHotspot hotspotSaveTemp = NexHotspot(PAGE_TEMP_SETTINGS_ID, HOTSPOT_TEMP_SETTINGS_SAVE, "");
NexHotspot hotspotSavePitmaster = NexHotspot(PAGE_PITMASTER_SETTINGS_ID, HOTSPOT_PITMASTER_SETTINGS_SAVE, "");
NexHotspot hotspotSaveSystem = NexHotspot(PAGE_SYSTEM_SETTINGS_ID, HOTSPOT_SYSTEM_SETTINGS_SAVE, "");

static NexButton menuWifiSettings = NexButton(PAGE_MENU_ID, BUTTON_MENU_WLAN_ID, "");
static NexButton menuSystemSettings = NexButton(PAGE_MENU_ID, BUTTON_MENU_SYSTEM_ID, "");
static NexButton menuPitmaster1Settings = NexButton(PAGE_PITM_MENU_ID, BUTTON_MENU_PITMASTER1_ID, "");
static NexButton menuPitmaster2Settings = NexButton(PAGE_PITM_MENU_ID, BUTTON_MENU_PITMASTER2_ID, "");
static NexButton wifiButtonLeft = NexButton(PAGE_WIFI_ID, BUTTON_WIFI_BUTTON_LEFT_ID, "");
static NexButton wifiButtonRight = NexButton(PAGE_WIFI_ID, BUTTON_WIFI_BUTTON_RIGHT_ID, "");
static NexText wifiButtonConnect = NexText(PAGE_WIFI_ID, TEXT_WIFI_CONNECT_ID, "");

NexTouch *nex_listen_list[] = {
    &nexTemperatures[0],
    &nexTemperatures[1],
    &nexTemperatures[2],
    &nexTemperatures[3],
    &nexTemperatures[4],
    &nexTemperatures[5],
    &tempButtonB,
    &tempButtonF,
    &alarmButton,
    &hotspotSaveTemp,
    &pitButtonChannel,
    &menuWifiSettings,
    &menuPitmaster1Settings,
    &menuPitmaster2Settings,
    &menuSystemSettings,
    &hotspotSaveSystem,
    &hotspotSavePitmaster,
    &wifiButtonLeft,
    &wifiButtonRight,
    &wifiButtonConnect,
    NULL};

uint32_t DisplayNextion::updateTemperature = 0u;
uint32_t DisplayNextion::updatePitmaster = 0u;
SystemBase *DisplayNextion::system = gSystem;
uint8_t DisplayNextion::serialTimeout = 0u;
boolean DisplayNextion::wifiScanInProgress = false;
ESPNexUpload DisplayNextion::nexUpload = ESPNexUpload(460800);
int8_t DisplayNextion::wifiIndex = 0u;
uint8_t DisplayNextion::tempPageIndex = 0u;

DisplayNextion::DisplayNextion()
{
  this->disabled = false;
  this->orientation = NEXTION_DIRECTION_ENUM;
}

void DisplayNextion::init()
{
  updateFromSPIFFS();

  xTaskCreatePinnedToCore(
      DisplayNextion::task,       /* Task function. */
      "DisplayNextion::task",     /* String with name of task. */
      10000,                      /* Stack size in bytes. */
      this,                       /* Parameter passed as input of the task */
      TASK_PRIORITY_DISPLAY_TASK, /* Priority of the task. */
      NULL,                       /* Task handle. */
      1);                         /* CPU Core */
}

boolean DisplayNextion::initDisplay()
{
  boolean didInit = false;

  if (this->disabled)
  {
    Serial.printf("DisplayNextion::init: display disabled\n");
    return true;
  }

  if (nexUpload.getBaudrate())
  {
    boolean cmdFinished;
    sendCommand("bkcmd=1");
    cmdFinished = recvRetCommandFinished();

    this->modelName = nexUpload.getModel().substring(0u, 10u);
    Serial.printf("Nextion model: %s\n", this->modelName.c_str());

    if (false == cmdFinished)
    {
      Serial.printf("cmdFinished: %d\n", cmdFinished);
      return false;
    }

    if (getCurrentPageNumber() != PAGE_BOOT_ID)
    {
      sendCommand("page 0");
    }

    setCounts();

    for (uint8_t i = 0; i < NEXTION_TEMPERATURES_PER_PAGE; i++)
    {
      nexTemperatures[i].attachPop(DisplayNextion::showTemperatureSettings, (void *)i);
    }

    tempButtonB.attachPop(DisplayNextion::navigateTemperature, (void *)BUTTON_TEMPB_ID);
    tempButtonF.attachPop(DisplayNextion::navigateTemperature, (void *)BUTTON_TEMPF_ID);
    alarmButton.attachPop(DisplayNextion::acknowledgeAlarm, NULL);
    menuWifiSettings.attachPop(DisplayNextion::enterWifiSettingsPage, this);
    menuSystemSettings.attachPop(DisplayNextion::enterSystemSettingsPage, this);
    menuPitmaster1Settings.attachPop(DisplayNextion::enterPitmasterSettingsPage, this);
    menuPitmaster2Settings.attachPop(DisplayNextion::enterPitmasterSettingsPage, this);

    // register for all temperature callbacks
    system->temperatures.registerCallback(temperatureUpdateCb, this);

    // register for all pitmaster callbacks
    for (uint8_t i = 0; i < system->pitmasters.count(); i++)
    {
      Pitmaster *pitmaster = system->pitmasters[i];
      if (pitmaster != NULL)
      {
        pitmaster->registerCallback(pitmasterUpdateCb, this);
      }
    }

    setSymbols(true);
    updateTemperaturePage(true);
    sendCommand("page temp_main");

    // restore display timeout
    String setTimeout = "thsp=" + String(this->timeout);
    sendCommand(setTimeout.c_str());

    didInit = true;
  }

  return didInit;
}

void DisplayNextion::updateTemperaturePage(boolean forceUpdate)
{
  static uint32_t activeBitsOld = 0u;

  uint8_t visibleCount = 0u;
  uint32_t activeBits = system->temperatures.getActiveBits();
  boolean updatePage = forceUpdate;
  uint32_t skippedTemperatures = 0u;

  if ((activeBits != activeBitsOld) || (UPDATE_ALL == updateTemperature) || (UPDATE_ALL == updatePitmaster))
    updatePage = true;

  if (updatePage)
  {
    if (getCurrentPageNumber() == PAGE_TEMP_MAIN_ID)
      sendCommand("page temp_load");

    uint32_t numOfTemperatures = system->temperatures.getActiveCount();
    numOfTemperatures = (0u == numOfTemperatures) ? system->temperatures.count() : numOfTemperatures;
    uint8_t numOfPages = (numOfTemperatures / NEXTION_TEMPERATURES_PER_PAGE) + 1u;
    // check if page index is still valid
    tempPageIndex = (tempPageIndex < numOfPages) ? tempPageIndex : numOfPages - 1u;
  }

  activeBitsOld = activeBits;

  // set all active bits when no temperature is active
  activeBits = (0u == activeBits) ? ((1 << system->temperatures.count()) - 1u) : activeBits;

  for (uint8_t i = 0; (i < system->temperatures.count()) && (visibleCount < NEXTION_TEMPERATURES_PER_PAGE); i++)
  {
    if (activeBits & (1u << i))
    {
      if (skippedTemperatures >= tempPageIndex * NEXTION_TEMPERATURES_PER_PAGE)
      {
        if (updatePage)
          setTemperatureAllItems(visibleCount, system->temperatures[i]);
        else if (updateTemperature & (1u << i))
          setTemperatureCurrent(visibleCount, system->temperatures[i]);

        if (updatePitmaster & (1u << i))
          setTemperaturePitmasterName(visibleCount, system->temperatures[i]);

        visibleCount++;
      }
      else
      {
        skippedTemperatures++;
      }
    }
  }

  if (updatePage)
  {
    NexVariable(DONT_CARE, DONT_CARE, "temp_main.Count").setValue(visibleCount);

    if (getCurrentPageNumber() == PAGE_TEMP_LOAD_ID)
      sendCommand("page temp_main");
  }

  updateTemperature = 0u;
  updatePitmaster = 0u;
}

void DisplayNextion::updatePitmasterChannel(void *ptr)
{
  uint32_t channelIndex = 0u;
  QUERY(NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.TempIndex").getValue(&channelIndex));
  channelIndex = ((channelIndex + 1u) < system->temperatures.count()) ? channelIndex + 1u : 0u;
  Serial.printf("channelIndex: %d\n", channelIndex);
  String channelName = system->temperatures[channelIndex]->getName();
  NexText(DONT_CARE, DONT_CARE, "pitm_settings.Channel").setText(channelName.c_str());
  NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.TempIndex").setValue(channelIndex);
}

void DisplayNextion::task(void *parameter)
{
  DisplayNextion *display = (DisplayNextion *)parameter;

  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint32_t bootScreenTimeout = 200u; // 1s
  while (bootScreenTimeout || display->system->isInitDone() != true)
  {
    vTaskDelay(10);
    if (bootScreenTimeout)
      bootScreenTimeout--;
  }

  while (display->initDisplay() == false)
    vTaskDelay(1000);

  for (;;)
  {
    display->update();
    // Wait for the next cycle.
    vTaskDelay(TASK_CYCLE_TIME_DISPLAY_TASK);
  }
}

uint8_t DisplayNextion::getCurrentPageNumber()
{
  uint8_t data[5] = {0u};
  uint8_t pageNumber = NEXTION_INVALID_PAGE;

  sendCommand("sendme");

  nexSerial.setTimeout(NEXTION_SERIAL_TIMEOUT);

  if (sizeof(data) != nexSerial.readBytes((char *)data, sizeof(data)))
  {
    return NEXTION_INVALID_PAGE;
  }

  if (data[0] == NEXTION_RETURN_CURRENT_PAGE_ID && data[2] == 0xFFu && data[3] == 0xFFu && data[4] == 0xFFu)
  {
    pageNumber = data[1];
  }

  return pageNumber;
}

void DisplayNextion::temperatureUpdateCb(uint8_t index, TemperatureBase *temperature, boolean settingsChanged, void *userData)
{
  DisplayNextion *displayNextion = (DisplayNextion *)userData;

  updateTemperature |= (true == settingsChanged) ? UPDATE_ALL : (1u << index);
}

void DisplayNextion::pitmasterUpdateCb(Pitmaster *pitmaster, boolean settingsChanged, void *userData)
{
  TemperatureBase *temperature = pitmaster->getAssignedTemperature();

  updatePitmaster |= (true == settingsChanged) ? UPDATE_ALL : (1u << temperature->getGlobalIndex());
}

void DisplayNextion::update()
{
  static uint8_t updateInProgress = false;
  static boolean wakeup = false;

  if (this->disabled)
    return;

  if (gSystem->otaUpdate.isUpdateInProgress())
  {
    if (false == updateInProgress)
    {
      updateInProgress = true;
      sendCommand("page boot");
    }
    return;
  }

  nexLoop(nex_listen_list);

  // check if we came from sleep
  if (getCurrentPageNumber() == PAGE_BOOT_ID)
  {
    updateTemperaturePage(true);
    sendCommand("page temp_main");
  }

  updateTemperaturePage();
  setSymbols();
  updateWifiSettingsPage();
}

void DisplayNextion::showTemperatureSettings(void *ptr)
{
  char text[20] = "";
  char item[20] = "";
  String Number = "0";
  uint8_t tempIndex;

  uint32_t nexIndex = reinterpret_cast<uint32_t>(ptr);

  memset(text, 0u, sizeof(text));
  sprintf(item, "temp_main.%s%d", "Number", nexIndex);
  NexText(DONT_CARE, DONT_CARE, item).getText(text, sizeof(text));
  Number = String(text);
  Number.replace("#", "");
  tempIndex = Number.toInt() - 1u;

  if (tempIndex >= system->temperatures.count())
    return;

  TemperatureBase *temperature = system->temperatures[tempIndex];

  // Name
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Name").setText(temperature->getName().c_str());
  // Min
  sprintf(text, "%d", (int32_t)temperature->getMinValue());
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Min").setText(text);
  // Max
  sprintf(text, "%d", (int32_t)temperature->getMaxValue());
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Max").setText(text);
  // Type
  NexVariable(DONT_CARE, DONT_CARE, "temp_settings.Type").setValue(temperature->getType());
  // Color
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Color").Set_background_color_bco(htmlColorToRgb565(temperature->getColor()));
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Color").Set_font_color_pco(htmlColorToRgb565(temperature->getColor()));
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Color").setText(temperature->getColor().c_str());

  // Alarm
  switch (temperature->getAlarmSetting())
  {
  case AlarmViaPush:
    NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Push").setValue(1u);
    NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Summer").setValue(0u);
    break;
  case AlarmViaSummer:
    NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Push").setValue(0u);
    NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Summer").setValue(1u);
    break;
  case AlarmAll:
    NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Push").setValue(1u);
    NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Summer").setValue(1u);
    break;
  default:
    NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Push").setValue(0u);
    NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Summer").setValue(0u);
    break;
  }

  // Fixed
  NexVariable(DONT_CARE, DONT_CARE, "temp_settings.Fixed").setValue(temperature->isFixedSensor());

  hotspotSaveTemp.attachPop(DisplayNextion::saveTemperatureSettings, temperature);

  sendCommand("page temp_settings");
}

void DisplayNextion::saveTemperatureSettings(void *ptr)
{
  char text[20] = "";
  char item[20] = "";
  char command[20] = "";
  uint32_t value = 0xFF;
  int32_t alarmSetting;

  TemperatureBase *temperature = (TemperatureBase *)ptr;

  // Name
  memset(text, 0u, sizeof(text));
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Name").getText(text, sizeof(text));
  temperature->setName(text);
  // Min
  memset(text, 0u, sizeof(text));
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Min").getText(text, sizeof(text));
  temperature->setMinValue(atoi(text));
  // Max
  memset(text, 0u, sizeof(text));
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Max").getText(text, sizeof(text));
  temperature->setMaxValue(atoi(text));
  // Type
  QUERY(NexVariable(DONT_CARE, DONT_CARE, "temp_settings.Type").getValue(&value));
  temperature->setType(value);
  // Color
  memset(text, 0u, sizeof(text));
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Color").getText(text, sizeof(text));
  temperature->setColor(text);

  // Alarm
  NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Push").getValue(&value);
  alarmSetting |= value;
  NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Summer").getValue(&value);
  alarmSetting |= (value << 1u);
  temperature->setAlarmSetting((AlarmSetting)alarmSetting);

  updateTemperaturePage(true);
  sendCommand("page temp_main");

  // save config
  system->temperatures.saveConfig();
}

void DisplayNextion::navigateTemperature(void *ptr)
{
  uint32_t buttonId = reinterpret_cast<uint32_t>(ptr);
  int8_t newPageIndex = (BUTTON_TEMPB_ID == buttonId) ? ((int8_t)tempPageIndex - 1) : ((int8_t)tempPageIndex + 1);
  uint32_t numOfTemperatures = system->temperatures.getActiveCount();

  numOfTemperatures = (0u == numOfTemperatures) ? system->temperatures.count() : numOfTemperatures;
  uint8_t numOfPages = (numOfTemperatures / NEXTION_TEMPERATURES_PER_PAGE) + 1u;

  if (newPageIndex < 0)
  {
    newPageIndex = numOfPages - 1u;
  }
  else if (newPageIndex >= numOfPages)
  {
    newPageIndex = 0u;
  }

  if (tempPageIndex != newPageIndex)
  {
    tempPageIndex = newPageIndex;
    updateTemperaturePage(true);
  }
}

void DisplayNextion::acknowledgeAlarm(void *ptr)
{
  system->temperatures.acknowledgeAlarm();
}

void DisplayNextion::saveSystemSettings(void *ptr)
{
  char unit[20] = "";
  uint32_t value = 0u;
  DisplayNextion *display = (DisplayNextion *)ptr;

  memset(unit, 0u, sizeof(unit));
  NexText(DONT_CARE, DONT_CARE, "Unit").getText(unit, sizeof(unit));

  if (String(unit) == "Fahrenheit")
  {
    system->temperatures.setUnit(TemperatureUnit::Fahrenheit);
  }
  else if (String(unit) == "Celsius")
  {
    system->temperatures.setUnit(TemperatureUnit::Celsius);
  }

  QUERY(NexVariable(DONT_CARE, DONT_CARE, "timeoutV").getValue(&value));
  display->timeout = value;

  display->saveConfig();

  sendCommand("page menu_main");
}

void DisplayNextion::enterWifiSettingsPage(void *ptr)
{
  char ssid[33] = "";

  memset(ssid, 0u, sizeof(ssid));
  NexText(DONT_CARE, DONT_CARE, "wifi_settings.Wifi").getText(ssid, sizeof(ssid));

  if (strlen(ssid))
  {
    /* Do nothing */
    /* Keyboard abort during entering of password */
    /* Keep scan */
    NexText(DONT_CARE, DONT_CARE, "wifi_settings.Wifi").setText("");
    NexText(DONT_CARE, DONT_CARE, "SSID").setFont(1u);
    NexText(DONT_CARE, DONT_CARE, "SSID").setText(WiFi.SSID(wifiIndex).c_str());
  }
  else if (false == wifiScanInProgress)
  {
    Serial.printf("DisplayNextion::enterWifiSettingsPage: scan network\n");
    WiFi.scanDelete();
    WiFi.scanNetworks(true);
    wifiScanInProgress = true;
  }
}

void DisplayNextion::enterSystemSettingsPage(void *ptr)
{
  DisplayNextion *display = (DisplayNextion *)ptr;
  TemperatureUnit temperatureUnit = system->temperatures.getUnit();

  if (TemperatureUnit::Fahrenheit == temperatureUnit)
  {
    NexText(DONT_CARE, DONT_CARE, "Unit").setText("Fahrenheit");
  }
  else if (TemperatureUnit::Celsius == temperatureUnit)
  {
    NexText(DONT_CARE, DONT_CARE, "Unit").setText("Celsius");
  }

  NexVariable(DONT_CARE, DONT_CARE, "timeoutV").setValue(display->timeout);

  hotspotSaveSystem.attachPop(DisplayNextion::saveSystemSettings, ptr);
  sendCommand("page sys_settings");
}

void DisplayNextion::enterPitmasterSettingsPage(void *ptr)
{
  char item[30];
  char text[20];
  uint32_t id = 0u;

  QUERY(NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.PitmasterId").getValue(&id));

  if (id < system->pitmasters.count())
  {
    // Type
    NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.TypeIndex").setValue(system->pitmasters[id]->getType());

    // Profile
    for (uint8_t i = 0; i < system->getPitmasterProfileCount(); i++)
    {
      sprintf(item, "pitm_settings.Profile%d", i);
      NexVariable(DONT_CARE, DONT_CARE, item).setText(system->getPitmasterProfile(i)->name.c_str());
    }

    NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.ProfileIdx").setValue(system->pitmasters[id]->getAssignedProfile()->id);

    // Value
    sprintf(text, "%d", (int)system->pitmasters[id]->getValue());
    NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.Value").setText(text);

    // Channel
    NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.TempIndex").setValue(system->pitmasters[id]->getAssignedTemperature()->getGlobalIndex());
    NexText(DONT_CARE, DONT_CARE, "pitm_settings.Channel").setText(system->pitmasters[id]->getAssignedTemperature()->getName().c_str());

    // Temperature
    sprintf(text, "%d", (int)system->pitmasters[id]->getTargetTemperature());
    NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.Temperature").setText(text);

    hotspotSavePitmaster.attachPop(DisplayNextion::savePitmasterSettings, system);
    pitButtonChannel.attachPop(DisplayNextion::updatePitmasterChannel, NULL);
    sendCommand("page pitm_settings");
  }
}

void DisplayNextion::savePitmasterSettings(void *ptr)
{
  char text[20] = "";
  char command[20] = "";
  uint32_t value = 0u;
  PitmasterType type = pm_off;
  uint32_t id = 0u;

  QUERY(NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.PitmasterId").getValue(&id));

  if (id < system->pitmasters.count())
  {

    // Type
    NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.TypeIndex").getValue(&value);
    Serial.printf("TypeIndex: %d\n", value);
    type = (PitmasterType)value;
    system->pitmasters[id]->setType((PitmasterType)type);

    if (pm_off != type)
    {
      // Profile
      QUERY(NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.ProfileIdx").getValue(&value));
      Serial.printf("ProfileIdx: %d\n", value);
      system->pitmasters[id]->assignProfile(system->getPitmasterProfile(value));
    }

    if (pm_manual == type)
    {
      // Value
      memset(text, 0u, sizeof(text));
      NexText(DONT_CARE, DONT_CARE, "pitm_settings.Value").getText(text, sizeof(text));
      Serial.printf("Value: %s\n", text);
      system->pitmasters[id]->setValue(atoi(text));
    }

    if (pm_auto == type)
    {
      // Channel
      QUERY(NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.TempIndex").getValue(&value));
      Serial.printf("TempIndex: %d\n", value);
      system->pitmasters[id]->assignTemperature(system->temperatures[value]);

      // Temperature
      memset(text, 0u, sizeof(text));
      NexText(DONT_CARE, DONT_CARE, "pitm_settings.Temperature").getText(text, sizeof(text));
      Serial.printf("Temperature: %s\n", text);
      system->pitmasters[id]->setTargetTemperature(atoi(text));
    }

    sendCommand("page menu_main");
    system->pitmasters.saveConfig();
  }
}

void DisplayNextion::updateWifiSettingsPage()
{
  if ((WiFi.scanComplete() > 0) && (true == wifiScanInProgress))
  {
    wifiIndex = 0u;
    NexText(DONT_CARE, DONT_CARE, "SSID").setFont(1u);
    NexText(DONT_CARE, DONT_CARE, "SSID").setText(WiFi.SSID(wifiIndex).c_str());
    wifiScanInProgress = false;
    wifiButtonLeft.attachPop(DisplayNextion::wifiNextLeft, system);
    wifiButtonRight.attachPop(DisplayNextion::wifiNextRight, system);
    wifiButtonConnect.attachPop(DisplayNextion::wifiConnect, system);
  }
}

void DisplayNextion::wifiNextLeft(void *ptr)
{
  wifiIndex--;
  wifiIndex = (wifiIndex < 0) ? WiFi.scanComplete() - 1 : wifiIndex;
  NexText(DONT_CARE, DONT_CARE, "SSID").setText(WiFi.SSID(wifiIndex).c_str());
}

void DisplayNextion::wifiNextRight(void *ptr)
{
  wifiIndex++;
  wifiIndex = (wifiIndex < WiFi.scanComplete()) ? wifiIndex : 0;
  NexText(DONT_CARE, DONT_CARE, "SSID").setText(WiFi.SSID(wifiIndex).c_str());
}

void DisplayNextion::wifiConnect(void *ptr)
{
  char ssid[33] = "";
  char password[64] = "";

  memset(ssid, 0u, sizeof(ssid));
  memset(password, 0u, sizeof(password));

  NexText(DONT_CARE, DONT_CARE, "wifi_settings.Wifi").getText(ssid, sizeof(ssid));
  NexText(DONT_CARE, DONT_CARE, "wifi_settings.Wifi").setText("");
  NexText(DONT_CARE, DONT_CARE, "wifi_settings.Password").getText(password, sizeof(password));
  NexText(DONT_CARE, DONT_CARE, "wifi_settings.Password").setText("");
  Serial.printf("%s, %s\n", ssid, password);

  if (strlen(ssid) && strlen(password))
  {
    system->wlan.addCredentials(ssid, password);
  }
}

void DisplayNextion::setCounts()
{
  NexVariable(DONT_CARE, DONT_CARE, "pitm_menu.Count").setValue(system->pitmasters.count());
}

void DisplayNextion::setTemperatureAllItems(uint8_t nexIndex, TemperatureBase *temperature)
{
  if (system->pitmasters.getActivePitmaster(temperature))
  {
    setTemperaturePitmasterName(nexIndex, temperature);
  }
  else
  {
    setTemperatureName(nexIndex, temperature);
  }

  setTemperatureColor(nexIndex, temperature);
  setTemperatureMin(nexIndex, temperature);
  setTemperatureMax(nexIndex, temperature);
  setTemperatureNumber(nexIndex, temperature);
  setTemperatureCurrent(nexIndex, temperature);
}

void DisplayNextion::setTemperatureColor(uint8_t nexIndex, TemperatureBase *temperature)
{
  char text[10] = "";
  char item[20];

  sprintf(item, "temp_main.%s%d", "Color", nexIndex);

  NexText(DONT_CARE, DONT_CARE, item).Set_background_color_bco(htmlColorToRgb565(temperature->getColor()));
}

void DisplayNextion::setTemperatureName(uint8_t nexIndex, TemperatureBase *temperature)
{
  char text[10] = "";
  char item[20];

  sprintf(item, "temp_main.%s%d", "Name", nexIndex);

  NexText(DONT_CARE, DONT_CARE, item).setText(temperature->getName().c_str());
}

void DisplayNextion::setTemperaturePitmasterName(uint8_t nexIndex, TemperatureBase *temperature)
{
  char text[20];
  char item[20];
  Pitmaster *pitmaster = system->pitmasters.getActivePitmaster(temperature);

  if (pitmaster != NULL)
  {
    if (pitmaster->getType() == PitmasterType::pm_auto)
    {
      sprintf(text, "P%d:%i\xb0/%d%%", pitmaster->getGlobalIndex() + 1u, (int)pitmaster->getTargetTemperature(), (uint8_t)pitmaster->getValue());
      sprintf(item, "temp_main.%s%d", "Name", nexIndex);
      NexText(DONT_CARE, DONT_CARE, item).setText(text);
    }
  }
}

void DisplayNextion::setTemperatureMin(uint8_t nexIndex, TemperatureBase *temperature)
{
  char text[10] = "";
  char item[20];

  sprintf(item, "temp_main.%s%d", "Min", nexIndex);

  sprintf(text, "%d\xb0", (int32_t)temperature->getMinValue());

  NexText(DONT_CARE, DONT_CARE, item).setText(text);
}

void DisplayNextion::setTemperatureMax(uint8_t nexIndex, TemperatureBase *temperature)
{
  char text[10] = "";
  char item[20];

  sprintf(item, "temp_main.%s%d", "Max", nexIndex);

  sprintf(text, "%d\xb0", (int32_t)temperature->getMaxValue());

  NexText(DONT_CARE, DONT_CARE, item).setText(text);
}

void DisplayNextion::setTemperatureNumber(uint8_t nexIndex, TemperatureBase *temperature)
{
  char text[10] = "";
  char item[20];

  sprintf(item, "temp_main.%s%d", "Number", nexIndex);

  sprintf(text, "#%d", (int32_t)temperature->getGlobalIndex() + 1);

  NexText(DONT_CARE, DONT_CARE, item).setText(text);
}

void DisplayNextion::setTemperatureCurrent(uint8_t nexIndex, TemperatureBase *temperature)
{
  char text[10] = "OFF";
  char item[20];

  sprintf(item, "temp_main.%s%d", "Current", nexIndex);

  if (temperature->getValue() != INACTIVEVALUE)
    sprintf(text, "%.1lf\xb0%c", temperature->getValue(), (char)system->temperatures.getUnit());

  NexText(DONT_CARE, DONT_CARE, item).setText(text);
  NexText(DONT_CARE, DONT_CARE, item).Set_font_color_pco(AlarmColorMap[temperature->getAlarmStatus()]);
}

void DisplayNextion::setSymbols(boolean forceUpdate)
{
  boolean newHasAlarm = system->temperatures.hasAlarm();
  WifiState newWifiState = system->wlan.getWifiState();
  WifiStrength newWifiStrength = system->wlan.getSignalStrength();
  char wifiSymbol = 'I';
  static uint8_t cloudState = system->cloud.state;
  static boolean hasAlarm = newHasAlarm;
  static WifiState wifiState = newWifiState;
  static WifiStrength wifiStrength = newWifiStrength;
  static uint32_t debounceWifiSymbol = millis();
  static boolean delayApSymbol = true;

  if ((cloudState != system->cloud.state) || forceUpdate)
  {
    NexButton(DONT_CARE, DONT_CARE, "temp_main.Cloud").setText((system->cloud.state != 2) ? "" : "h");
    cloudState = system->cloud.state;
  }

  if ((hasAlarm != newHasAlarm) || forceUpdate)
  {
    NexButton(DONT_CARE, DONT_CARE, "temp_main.Alarm").setText((newHasAlarm) ? "O" : "");
    hasAlarm = newHasAlarm;
  }

  if (delayApSymbol && (millis() > 10000u))
  {
    forceUpdate = true;
    delayApSymbol = false;
  }

  if ((wifiStrength != newWifiStrength) || forceUpdate)
  {
    switch (newWifiStrength)
    {
    case WifiStrength::High:
      wifiSymbol = 'I';
      break;
    case WifiStrength::Medium:
      wifiSymbol = 'H';
      break;
    case WifiStrength::Low:
      wifiSymbol = 'G';
      break;
    default:
      wifiSymbol = '\0';
      break;
    }
    if ((millis() - debounceWifiSymbol) >= 1000u)
    {
      wifiStrength = newWifiStrength;
      forceUpdate = true;
      debounceWifiSymbol = millis();
    }
  }

  if ((wifiState != newWifiState) || forceUpdate)
  {
    String info;

    switch (newWifiState)
    {
    case WifiState::SoftAPNoClient:
      if (delayApSymbol)
        break;
      NexButton(DONT_CARE, DONT_CARE, "temp_main.Wifi").setText("l");
      NexVariable(DONT_CARE, DONT_CARE, "wifi_info.WifiName").setText(system->wlan.getAccessPointName().c_str());
      NexVariable(DONT_CARE, DONT_CARE, "wifi_info.CustomInfo").setText("12345678");
      break;
    case WifiState::SoftAPClientConnected:
      if (delayApSymbol)
        break;
      NexButton(DONT_CARE, DONT_CARE, "temp_main.Wifi").setText("l");
      NexVariable(DONT_CARE, DONT_CARE, "wifi_info.WifiName").setText("");
      NexVariable(DONT_CARE, DONT_CARE, "wifi_info.CustomInfo").setText("http://192.168.66.1");
      break;
    case WifiState::ConnectedToSTA:
      info = "http://" + WiFi.localIP().toString();
      NexButton(DONT_CARE, DONT_CARE, "temp_main.Wifi").setText(String(wifiSymbol).c_str());
      NexText(DONT_CARE, DONT_CARE, "wifi_info.WifiName").setText(WiFi.SSID().c_str());
      NexText(DONT_CARE, DONT_CARE, "wifi_info.CustomInfo").setText(info.c_str());
      break;
    case WifiState::ConnectingToSTA:
    case WifiState::AddCredentials:
      break;
    default:
      NexButton(DONT_CARE, DONT_CARE, "temp_main.Wifi").setText("");
      NexText(DONT_CARE, DONT_CARE, "wifi_info.WifiName").setText("");
      NexText(DONT_CARE, DONT_CARE, "wifi_info.CustomInfo").setText("");
      break;
    }
    wifiState = newWifiState;
  }
}

uint32_t DisplayNextion::htmlColorToRgb565(String htmlColor)
{
  // Get rid of '#' and convert it to integer
  uint32_t number = (uint32_t)strtol(htmlColor.substring(1).c_str(), NULL, 16);

  // Split them up into r, g, b values
  uint8_t r = number >> 16;
  uint8_t g = number >> 8 & 0xFF;
  uint8_t b = number & 0xFF;

  return ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
}

void DisplayNextion::calibrate()
{
  sendCommand("touch_j");
}

String DisplayNextion::getUpdateName()
{
  String updateName = "";

  if (this->modelName != "")
  {
    updateName += this->modelName + String("-") + String((uint16_t)this->orientation);
  }

  return updateName;
}

void DisplayNextion::updateFromSPIFFS()
{
  if (!SPIFFS.begin(false))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // check for nextion files
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  uint8_t numOfNexFiles = 0u;

  while (file)
  {
    if (String(file.name()).endsWith(".tft.zlib"))
    {
      Serial.print("NEXTION FILE: ");
      Serial.println(file.name());
      numOfNexFiles++;
    }

    file = root.openNextFile();
  }

  if (0u == numOfNexFiles)
  {
    Serial.println("No Nextion update available");
    return;
  }

  // connect to display
  if (!nexUpload.getBaudrate())
  {
    Serial.println("Cannot connect to Nextion");
    return;
  }

  // create filename from model and direction string
  String nextionFileName = "/" + nexUpload.getModel().substring(0u, 10u) + NEXTION_DIRECTION_STRING + ".tft.zlib";

  Serial.printf("Nextion update file name: %s\n", nextionFileName.c_str());

  // check if needed nextion file is available
  if (SPIFFS.exists(NEXTION_SPIFFS_UPDATE_FILENAME))
  {
    // file from OTA
    Serial.printf("Nextion update from OTA: %s\n", NEXTION_SPIFFS_UPDATE_FILENAME);
    nextionFileName = NEXTION_SPIFFS_UPDATE_FILENAME;
  }
  else if (!SPIFFS.exists(nextionFileName.c_str()))
  {
    // file from SPIFFS upload
    Serial.println("Nextion update file not available");
    return;
  }

  // open nextion file
  File nextionFile = SPIFFS.open(nextionFileName, FILE_READ);

  if (!nextionFile)
  {
    Serial.println("Error opening nextion file");
    return;
  }

  std::unique_ptr<tinfl_decompressor> decomp(new tinfl_decompressor);
  std::unique_ptr<uint8_t[]> outBuffer(new uint8_t[OUTBUFFER_SIZE]);
  std::unique_ptr<uint8_t[]> inBuffer(new uint8_t[INBUFFER_SIZE]);
  size_t inPosition = 0u;
  size_t outPosition = 0u;
  size_t outBytes = 0u;
  size_t inBytes = 0u;
  size_t totalBytes = 0u;

  if (!decomp.get() || !outBuffer.get() || !inBuffer.get())
  {
    Serial.println("Cannot allocate memory");
  }

  tinfl_init(decomp.get());

  // first decompression for getting the final size
  while (nextionFile.available())
  {
    inBytes = nextionFile.readBytes((char *)inBuffer.get(), INBUFFER_SIZE);
    outBytes = OUTBUFFER_SIZE;
    mz_uint32 flags = TINFL_FLAG_PARSE_ZLIB_HEADER | ((nextionFile.position() < nextionFile.size()) ? TINFL_FLAG_HAS_MORE_INPUT : 0u);
    tinfl_status status = tinfl_decompress(decomp.get(), (const mz_uint8 *)inBuffer.get(), &inBytes, (uint8_t *)outBuffer.get(), (mz_uint8 *)outBuffer.get(), &outBytes, flags);
    if (0u == outBytes)
      break;
    totalBytes += outBytes;
    inPosition += inBytes;
    nextionFile.seek(inPosition);
    Serial.printf("Decompressed %d bytes\n", totalBytes);
  }

  if (!nexUpload.prepareUpload(totalBytes))
  {
    Serial.println("Prepare Nextion upload failed!");
    return;
  }

  // reset data for second decompression
  inPosition = 0u;
  outPosition = 0u;
  outBytes = 0u;
  inBytes = 0u;
  totalBytes = 0u;

  decomp.reset(new tinfl_decompressor);
  tinfl_init(decomp.get());
  nextionFile.seek(0u);

  // second decompression for uploading the file
  while (nextionFile.available())
  {
    inBytes = nextionFile.readBytes((char *)inBuffer.get(), INBUFFER_SIZE);
    outBytes = OUTBUFFER_SIZE;
    mz_uint32 flags = TINFL_FLAG_PARSE_ZLIB_HEADER | ((nextionFile.position() < nextionFile.size()) ? TINFL_FLAG_HAS_MORE_INPUT : 0u);
    tinfl_status status = tinfl_decompress(decomp.get(), (const mz_uint8 *)inBuffer.get(), &inBytes, (mz_uint8 *)outBuffer.get(), (mz_uint8 *)outBuffer.get(), &outBytes, flags);
    if (0u == outBytes)
      break;
    totalBytes += outBytes;
    inPosition += inBytes;
    nextionFile.seek(inPosition);
    Serial.printf("Uploading %d bytes\n", totalBytes);
    if (!nexUpload.upload(outBuffer.get(), outBytes))
    {
      Serial.println("Prepare Nextion upload failed!");
      return;
    }
  }

  nexUpload.end();

  //TODO: test connection again?

  // delete all nextion files
  root = SPIFFS.open("/");
  file = root.openNextFile();

  while (file)
  {

    if (String(file.name()).endsWith(".tft.zlib"))
    {
      Serial.print("NEXTION REMOVE: ");
      Serial.println(file.name());
      SPIFFS.remove(file.name());
    }

    file = root.openNextFile();
  }

  SPIFFS.end();
}