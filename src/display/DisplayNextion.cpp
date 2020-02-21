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

extern "C" {
#include <rom/miniz.h>
}

#define OUTBUFFER_SIZE (32u*1024u)
#define INBUFFER_SIZE  (32u*1024u)
#define NEXTION_SPIFFS_UPDATE_FILENAME "/nextion.tft.zlib"

#define NEXTION_TEMPERATURES_MAX 12u
#define NEXTION_TEMPERATURES_PER_PAGE 6u
#define UPDATE_ALL_TEMPERATURES 0xFFFFFFFFu
#define NEXTION_SERIAL_TIMEOUT 5u

#define PAGE_TEMP_INDEX (temperature->getGlobalIndex() / NEXTION_TEMPERATURES_PER_PAGE)
#define PAGE_TEMP_ITEM_INDEX (temperature->getGlobalIndex() % NEXTION_TEMPERATURES_PER_PAGE)
#define QUERY(a)      \
  serialTimeout = 5u; \
  while (((a) != true) && serialTimeout-- > 0u)

#define DONT_CARE 0u
#define NEXTION_COLOR_NO_ALARM 0xFFFFu
#define NEXTION_COLOR_MIN_ALARM 0x32F0u
#define NEXTION_COLOR_MAX_ALARM 0xF800u

uint16_t AlarmColorMap[3u] = {NEXTION_COLOR_NO_ALARM, NEXTION_COLOR_MIN_ALARM, NEXTION_COLOR_MAX_ALARM};

#define PAGE_TEMP0_MAIN_ID 1u
#define PAGE_TEMP1_MAIN_ID 2u
#define PAGE_MENU_ID 6u
#define PAGE_WIFI_ID 7u
#define HOTSPOT_TEMP0_ID 66u
#define HOTSPOT_TEMP1_ID 67u
#define HOTSPOT_TEMP2_ID 68u
#define HOTSPOT_TEMP3_ID 69u
#define HOTSPOT_TEMP4_ID 70u
#define HOTSPOT_TEMP5_ID 71u
#define BUTTON_MENU_WLAN_ID 2u
#define BUTTON_MENU_PITMASTER_ID 3u
#define TEXT_WIFI_CONNECT_ID 1u
#define BUTTON_WIFI_BUTTON_LEFT_ID 10u
#define BUTTON_WIFI_BUTTON_RIGHT_ID 11u

#define PAGE_TEMP_SETTINGS_ID 3u
#define PAGE_PITMASTER_SETTINGS 9u
#define HOTSPOT_TEMP_SETTINGS_SAVE 26u
#define HOTSPOT_PITMASTER_SETTINGS_SAVE 28u

NexHotspot nexTemperatures[NEXTION_TEMPERATURES_MAX] = {
    NexHotspot(PAGE_TEMP0_MAIN_ID, HOTSPOT_TEMP0_ID, ""),
    NexHotspot(PAGE_TEMP0_MAIN_ID, HOTSPOT_TEMP1_ID, ""),
    NexHotspot(PAGE_TEMP0_MAIN_ID, HOTSPOT_TEMP2_ID, ""),
    NexHotspot(PAGE_TEMP0_MAIN_ID, HOTSPOT_TEMP3_ID, ""),
    NexHotspot(PAGE_TEMP0_MAIN_ID, HOTSPOT_TEMP4_ID, ""),
    NexHotspot(PAGE_TEMP0_MAIN_ID, HOTSPOT_TEMP5_ID, ""),
    NexHotspot(PAGE_TEMP1_MAIN_ID, HOTSPOT_TEMP0_ID, ""),
    NexHotspot(PAGE_TEMP1_MAIN_ID, HOTSPOT_TEMP1_ID, ""),
    NexHotspot(PAGE_TEMP1_MAIN_ID, HOTSPOT_TEMP2_ID, ""),
    NexHotspot(PAGE_TEMP1_MAIN_ID, HOTSPOT_TEMP3_ID, ""),
    NexHotspot(PAGE_TEMP1_MAIN_ID, HOTSPOT_TEMP4_ID, ""),
    NexHotspot(PAGE_TEMP1_MAIN_ID, HOTSPOT_TEMP5_ID, ""),
};

NexHotspot hotspotSaveTemp = NexHotspot(PAGE_TEMP_SETTINGS_ID, HOTSPOT_TEMP_SETTINGS_SAVE, "");
NexHotspot hotspotSavePitmaster = NexHotspot(PAGE_PITMASTER_SETTINGS, HOTSPOT_PITMASTER_SETTINGS_SAVE, "");

static NexButton menuWifiSettings = NexButton(PAGE_MENU_ID, BUTTON_MENU_WLAN_ID, "");
static NexButton menuPitmasterSettings = NexButton(PAGE_MENU_ID, BUTTON_MENU_PITMASTER_ID, "");
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
    &nexTemperatures[6],
    &nexTemperatures[7],
    &nexTemperatures[8],
    &nexTemperatures[9],
    &nexTemperatures[10],
    &nexTemperatures[11],
    &hotspotSaveTemp,
    &menuWifiSettings,
    &menuPitmasterSettings,
    &hotspotSavePitmaster,
    &wifiButtonLeft,
    &wifiButtonRight,
    &wifiButtonConnect,
    NULL};

uint32_t DisplayNextion::updateTemperature = 0u;
SystemBase *DisplayNextion::system = gSystem;
uint8_t DisplayNextion::serialTimeout = 0u;
boolean DisplayNextion::wifiScanInProgress = false;
ESPNexUpload DisplayNextion::nexUpload = ESPNexUpload(460800);
int8_t DisplayNextion::wifiIndex = 0u;

DisplayNextion::DisplayNextion()
{
  this->disabled = false;
}

void DisplayNextion::init()
{
  updateFromSPIFFS();

  this->loadConfig();

  xTaskCreatePinnedToCore(
      DisplayNextion::task,   /* Task function. */
      "DisplayNextion::task", /* String with name of task. */
      10000,                  /* Stack size in bytes. */
      this,                   /* Parameter passed as input of the task */
      1,                      /* Priority of the task. */
      NULL,                   /* Task handle. */
      1);                     /* CPU Core */
}

boolean DisplayNextion::initDisplay()
{
  boolean didInit = false;

  if (this->disabled)
  {
    Serial.printf("DisplayNextion::init: display disabled\n");
    return true;
  }

  if (nexInit())
  {
    setTemperatureCount();

    for (int i = 0; i < NEXTION_TEMPERATURES_MAX; i++)
    {
      TemperatureBase *temperature = system->temperatures[i];
      if (temperature != NULL)
      {
        nexTemperatures[i].attachPop(DisplayNextion::showTemperatureSettings, temperature);
        setTemperatureAllItems(temperature);
      }
    }

    menuWifiSettings.attachPop(DisplayNextion::enterWifiSettingsPage, this);
    menuPitmasterSettings.attachPop(DisplayNextion::enterPitmasterSettingsPage, this);

    // register for all temperature callbacks
    for (uint8_t i = 0; i < system->temperatures.count(); i++)
    {
      TemperatureBase *temperature = system->temperatures[i];
      if (temperature != NULL)
      {
        temperature->registerCallback(temperatureUpdateCb, this);
      }
    }

    setSymbols(true);

    sendCommand("page temp_main0");
    didInit = true;
  }

  return didInit;
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
    // Wait for the next cycle.
    vTaskDelay(10);
    display->update();
  }
}

void DisplayNextion::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();
  json["disabled"] = this->disabled;
  Settings::write(kDisplay, json);
}

void DisplayNextion::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kDisplay, &jsonBuffer);

  if (json.success())
  {

    if (json.containsKey("disabled"))
      this->disabled = json["disabled"].as<boolean>();
  }
}

void DisplayNextion::temperatureUpdateCb(TemperatureBase *temperature, boolean settingsChanged, void *userData)
{
  DisplayNextion *displayNextion = (DisplayNextion *)userData;

  updateTemperature |= (true == settingsChanged) ? UPDATE_ALL_TEMPERATURES : (1u << temperature->getGlobalIndex());
}

void DisplayNextion::update()
{
  static uint8_t tempNavIndex = 0u;
  boolean updateAllTemperatures = false;

  if (this->disabled)
    return;

  nexLoop(nex_listen_list);

  if (wifiScanInProgress)
  {
    updateWifiSettingsPage();
  }

  setSymbols();

  if (UPDATE_ALL_TEMPERATURES == updateTemperature)
    updateAllTemperatures = true;

  for (uint8_t i = 0; i < NEXTION_TEMPERATURES_MAX; i++)
  {
    TemperatureBase *temperature = system->temperatures[i];

    if (temperature && updateAllTemperatures)
    {
      nexTemperatures[i].attachPop(DisplayNextion::showTemperatureSettings, temperature);
      setTemperatureAllItems(temperature);
    }
    else if (temperature && (this->updateTemperature & (1u << i)))
    {
      setTemperatureCurrent(temperature);
    }
  }

  this->updateTemperature = 0u;
}

void DisplayNextion::showTemperatureSettings(void *ptr)
{
  char text[10] = "";
  char item[20];

  TemperatureBase *temperature = (TemperatureBase *)ptr;

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
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Name").getText(text, sizeof(text));
  temperature->setName(text);
  // Min
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Min").getText(text, sizeof(text));
  temperature->setMinValue(atoi(text));
  // Max
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Max").getText(text, sizeof(text));
  temperature->setMaxValue(atoi(text));
  // Type
  QUERY(NexVariable(DONT_CARE, DONT_CARE, "temp_settings.Type").getValue(&value));
  temperature->setType(value);
  Serial.printf("type: %d\n", value);
  // Color
  NexText(DONT_CARE, DONT_CARE, "temp_settings.Color").getText(text, sizeof(text));
  temperature->setColor(text);

  // Alarm
  NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Push").getValue(&value);
  alarmSetting |= value;
  NexCheckbox(DONT_CARE, DONT_CARE, "temp_settings.Summer").getValue(&value);
  alarmSetting |= (value << 1u);
  temperature->setAlarmSetting((AlarmSetting)alarmSetting);

  setTemperatureAllItems(temperature);

  // Goto previous page
  NexText(DONT_CARE, DONT_CARE, "temp_main0.cp").getText(text, sizeof(text));
  sprintf(command, "page %s", text);
  sendCommand(command);

  // save config
  system->temperatures.saveConfig();
}

void DisplayNextion::enterWifiSettingsPage(void *ptr)
{
  char ssid[33] = "";

  NexText(DONT_CARE, DONT_CARE, "wifi_settings.Wifi").getText(ssid, sizeof(ssid));

  if(strlen(ssid))
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

void DisplayNextion::enterPitmasterSettingsPage(void *ptr)
{
  char item[30];
  char text[20];

  // Type
  NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.TypeIndex").setValue(system->pitmasters[0]->getType());

  // Profile
  for (uint8_t i = 0; i < system->getPitmasterProfileCount(); i++)
  {
    sprintf(item, "pitm_settings.Profile%d", i);
    NexVariable(DONT_CARE, DONT_CARE, item).setText(system->getPitmasterProfile(i)->name.c_str());
  }

  NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.ProfileIdx").setValue(system->pitmasters[0]->getAssignedProfile()->id);

  // Value
  sprintf(text, "%d", (int)system->pitmasters[0]->getValue());
  NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.Value").setText(text);

  // Channel
  NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.TempIndex").setValue(system->pitmasters[0]->getAssignedTemperature()->getGlobalIndex());

  // Temperature
  sprintf(text, "%d", (int)system->pitmasters[0]->getTargetTemperature());
  NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.Temperature").setText(text);

  hotspotSavePitmaster.attachPop(DisplayNextion::savePitmasterSettings, system);
  sendCommand("page pitm_settings");
}

void DisplayNextion::savePitmasterSettings(void *ptr)
{
  char text[20] = "";
  char command[20] = "";
  uint32_t value = 0u;
  PitmasterType type = pm_off;

  // Type
  NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.TypeIndex").getValue(&value);
  Serial.printf("TypeIndex: %d\n", value);
  type = (PitmasterType)value;
  system->pitmasters[0]->setType((PitmasterType)type);

  if (pm_off != type)
  {
    // Profile
    QUERY(NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.ProfileIdx").getValue(&value));
    Serial.printf("ProfileIdx: %d\n", value);
    system->pitmasters[0]->assignProfile(system->getPitmasterProfile(value));
  }

  if (pm_manual == type)
  {
    // Value
    NexText(DONT_CARE, DONT_CARE, "pitm_settings.Value").getText(text, sizeof(text));
    Serial.printf("Value: %s\n", text);
    system->pitmasters[0]->setValue(atoi(text));
  }

  if (pm_auto == type)
  {
    // Channel
    QUERY(NexVariable(DONT_CARE, DONT_CARE, "pitm_settings.TempIndex").getValue(&value));
    Serial.printf("TempIndex: %d\n", value);
    system->pitmasters[0]->assignTemperature(system->temperatures[value]);

    // Temperature
    NexText(DONT_CARE, DONT_CARE, "pitm_settings.Temperature").getText(text, sizeof(text));
    Serial.printf("Temperature: %s\n", text);
    system->pitmasters[0]->setTargetTemperature(atoi(text));
  }

  sendCommand("page menu_main");
  system->pitmasters.saveConfig();
}

void DisplayNextion::updateWifiSettingsPage()
{
  if(WiFi.scanComplete() > 0)
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

  NexText(DONT_CARE, DONT_CARE, "wifi_settings.Wifi").getText(ssid, sizeof(ssid));
  NexText(DONT_CARE, DONT_CARE, "wifi_settings.Wifi").setText("");
  NexText(DONT_CARE, DONT_CARE, "wifi_settings.Password").getText(password, sizeof(password));
  NexText(DONT_CARE, DONT_CARE, "wifi_settings.Password").setText("");
  Serial.printf("%s, %s\n", ssid, password);

  if(strlen(ssid) && strlen(password))
  {
    system->wlan.addCredentials(ssid, password);
  }
}

void DisplayNextion::setTemperatureCount()
{
  NexVariable(DONT_CARE, DONT_CARE, "temp_main0.Count").setValue(system->temperatures.count());
}

void DisplayNextion::setTemperatureAllItems(TemperatureBase *temperature)
{
  setTemperatureColor(temperature);
  setTemperatureName(temperature);
  setTemperatureMin(temperature);
  setTemperatureMax(temperature);
  setTemperatureCurrent(temperature);
}

void DisplayNextion::setTemperatureColor(TemperatureBase *temperature)
{
  char text[10] = "";
  char item[20];

  sprintf(item, "temp_main%d.%s%d", PAGE_TEMP_INDEX, "Color", PAGE_TEMP_ITEM_INDEX);

  NexText(DONT_CARE, DONT_CARE, item).Set_background_color_bco(htmlColorToRgb565(temperature->getColor()));
}

void DisplayNextion::setTemperatureName(TemperatureBase *temperature)
{
  char text[10] = "";
  char item[20];

  sprintf(item, "temp_main%d.%s%d", PAGE_TEMP_INDEX, "Name", PAGE_TEMP_ITEM_INDEX);

  NexText(DONT_CARE, DONT_CARE, item).setText(temperature->getName().c_str());
}

void DisplayNextion::setTemperatureMin(TemperatureBase *temperature)
{
  char text[10] = "";
  char item[20];

  sprintf(item, "temp_main%d.%s%d", PAGE_TEMP_INDEX, "Min", PAGE_TEMP_ITEM_INDEX);

  sprintf(text, "%d\xb0", (int32_t)temperature->getMinValue());

  NexText(DONT_CARE, DONT_CARE, item).setText(text);
}

void DisplayNextion::setTemperatureMax(TemperatureBase *temperature)
{
  char text[10] = "";
  char item[20];

  sprintf(item, "temp_main%d.%s%d", PAGE_TEMP_INDEX, "Max", PAGE_TEMP_ITEM_INDEX);

  sprintf(text, "%d\xb0", (int32_t)temperature->getMaxValue());

  NexText(DONT_CARE, DONT_CARE, item).setText(text);
}

void DisplayNextion::setTemperatureCurrent(TemperatureBase *temperature)
{
  char text[10] = "OFF";
  char item[20];

  sprintf(item, "temp_main%d.%s%d", PAGE_TEMP_INDEX, "Current", PAGE_TEMP_ITEM_INDEX);

  if (temperature->getValue() != INACTIVEVALUE)
    sprintf(text, "%.1lf\xb0%c", temperature->getValue(), (char)system->temperatures.getUnit());

  NexText(DONT_CARE, DONT_CARE, item).setText(text);
  NexText(DONT_CARE, DONT_CARE, item).Set_font_color_pco(AlarmColorMap[temperature->getAlarmStatus()]);
}

void DisplayNextion::setSymbols(boolean forceUpdate)
{
  boolean newHasAlarm = system->temperatures.hasAlarm();
  WifiState newWifiState = system->wlan.getWifiState();
  static uint8_t cloudState = system->cloud.state;
  static boolean hasAlarm = newHasAlarm;
  static WifiState wifiState = newWifiState;

  if (cloudState != system->cloud.state || forceUpdate)
  {
    NexButton(DONT_CARE, DONT_CARE, "temp_main0.Cloud").setText((system->cloud.state != 2) ? "" : "h");
    cloudState = system->cloud.state;
  }

  if (wifiState != newWifiState || forceUpdate)
  {
    switch(newWifiState)
    {
      case WifiState::SoftAPNoClient:
        NexButton(DONT_CARE, DONT_CARE, "temp_main0.Wifi").setText("l");
        NexText(DONT_CARE, DONT_CARE, "wifi_info.QrCode").setText("WIFI:S:MINI-AP;T:WPA;P:12345678;;");
        NexText(DONT_CARE, DONT_CARE, "wifi_info.Info").setText("AP: MINI-AP | PW: 12345678");
        break;
      case WifiState::SoftAPClientConnected:
        NexButton(DONT_CARE, DONT_CARE, "temp_main0.Wifi").setText("l");
        NexText(DONT_CARE, DONT_CARE, "wifi_info.QrCode").setText("http://192.168.66.1");
        NexText(DONT_CARE, DONT_CARE, "wifi_info.Info").setText("http://192.168.66.1");
        break;
      case WifiState::ConnectedToSTA:
        NexButton(DONT_CARE, DONT_CARE, "temp_main0.Wifi").setText("I");
        NexText(DONT_CARE, DONT_CARE, "wifi_info.QrCode").setText(String("http://" + WiFi.localIP().toString()).c_str());
        NexText(DONT_CARE, DONT_CARE, "wifi_info.Info").setText(String("http://" + WiFi.localIP().toString()).c_str());
        break;
      case WifiState::ConnectingToSTA:
      case WifiState::AddCredentials:
        break;
      default:
        NexButton(DONT_CARE, DONT_CARE, "temp_main0.Wifi").setText("");
        NexText(DONT_CARE, DONT_CARE, "wifi_info.QrCode").setText("");
        NexText(DONT_CARE, DONT_CARE, "wifi_info.Info").setText("");
        break;
    }
    wifiState = newWifiState;
  }

  if (hasAlarm != newHasAlarm || forceUpdate)
  {
    NexButton(DONT_CARE, DONT_CARE, "temp_main0.Alarm").setText((newHasAlarm) ? "O" : "");
    hasAlarm = newHasAlarm;
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

void DisplayNextion::updateFromSPIFFS()
{
  if (!SPIFFS.begin(false))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  if (!SPIFFS.exists(NEXTION_SPIFFS_UPDATE_FILENAME))
  {
    Serial.println("No nextion update available");
    return;
  }

  File nextionFile = SPIFFS.open(NEXTION_SPIFFS_UPDATE_FILENAME, FILE_READ);

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
    if(0u == outBytes) break;
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
    if(0u == outBytes) break;
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

  SPIFFS.remove(NEXTION_SPIFFS_UPDATE_FILENAME);
  SPIFFS.end();
}