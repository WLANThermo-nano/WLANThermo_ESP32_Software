/*************************************************** 
    Copyright (C) 2016  Steffen Ochs
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
#include <esp_pm.h>
#include <OLEDDisplay.h>
#include "DisplayOled.h"
#include "DisplayOledIcons.h"
#include "Settings.h"
#include "Version.h"

#define MAXBATTERYBAR 13u
#define OLIMITMIN 35.0
#define OLIMITMAX 200.0
#define ULIMITMINF 50.0
#define ULIMITMAXF 302.0
#define OLIMITMINF 95.0
#define OLIMITMAXF 392.0
#define DISPLAY_I2C_ADDRESS 0x3cu
#define LBUTTON_IO 14u
#define RBUTTON_IO 12u
#define OLED_TASK_CYCLE_TIME 10u   // 10ms
#define OLED_BOOT_SCREEN_TIME 100u // 100 * 10ms = 1s
#define OLED_FLASH_INTERVAL 50u    // 50 * 10ms = 500ms
#define OLED_WIFI_AP_DELAY 10000u
#define OLED_BATTERY_PERCENTAGE_DELAY 10000u
#define BUTTON_DEBOUNCE_TICKS 10u
#define BUTTON_CLICK_TICKS 200u
#define BUTTON_PRESS_TICKS 600u

enum class Frames
{
  Temperature,
  TemperatureSettings,
  PitmasterSettings,
  SystemSettings,
  NumOfFrames,
};

float DisplayOled::currentData = 0; // Zwischenspeichervariable
uint8_t DisplayOled::buttonMupi = 1u;
boolean DisplayOled::oledBlocked = false;
String alarmname[4] = {"off", "push", "summer", "all"};

SystemBase *DisplayOled::system = gSystem;
SH1106Wire DisplayOled::oled = SH1106Wire(DISPLAY_I2C_ADDRESS, SDA, SCL);
OLEDDisplayUi DisplayOled::ui = OLEDDisplayUi(&DisplayOled::oled);
FrameCallback DisplayOled::frames[] = {DisplayOled::drawTemp, DisplayOled::drawTempSettings, DisplayOled::drawPitmasterSettings, DisplayOled::drawSystemSettings};
OverlayCallback DisplayOled::overlays[] = {drawOverlayBar};
OneButton DisplayOled::lButton = OneButton(LBUTTON_IO, true, true);
OneButton DisplayOled::rButton = OneButton(RBUTTON_IO, true, true);
MenuItem DisplayOled::menuItem = MenuItem::Boot;
MenuMode DisplayOled::menuMode = MenuMode::Show;
uint8_t DisplayOled::currentChannel = 0u;
boolean DisplayOled::flashIndicator = false;

DisplayOled::DisplayOled()
{
}

void DisplayOled::init()
{
  xTaskCreatePinnedToCore(
      DisplayOled::task,   /* Task function. */
      "DisplayOled::task", /* String with name of task. */
      10000,               /* Stack size in bytes. */
      this,                /* Parameter passed as input of the task */
      1,                   /* Priority of the task. */
      NULL,                   /* Task handle. */
      1);                     /* CPU Core */
}

boolean DisplayOled::initDisplay()
{
  this->loadConfig();

  ui.setTargetFPS(30);
  ui.setFrames(frames, (uint8_t)Frames::NumOfFrames);
  ui.setOverlays(overlays, 1u);
  ui.setTimePerFrame(10000);
  ui.setTimePerTransition(300);
  ui.disableAutoTransition();
  ui.disableAllIndicators();
  ui.init();

  oled.flipScreenVertically();
  oled.clear();
  oled.display();
  drawConnect();

  //question.typ = NO;
  //question.con = 0;

  lButton.attachClick(this->lButtonClick);
  lButton.attachLongPressStart(this->lButtonLongClickStart);
  lButton.attachLongPressStop(this->lButtonLongClickEnd);
  lButton.attachDoubleClick(this->lButtonDoubleClick);
  lButton.attachDuringLongPress(this->lButtonLongClickOnGoing);
  lButton.setDebounceTicks(BUTTON_DEBOUNCE_TICKS);
  lButton.setClickTicks(BUTTON_CLICK_TICKS);
  lButton.setPressTicks(BUTTON_PRESS_TICKS);

  rButton.attachClick(this->rButtonClick);
  rButton.attachLongPressStart(this->rButtonLongClickStart);
  rButton.attachLongPressStop(this->rButtonLongClickEnd);
  rButton.attachDoubleClick(this->rButtonDoubleClick);
  rButton.attachDuringLongPress(this->rButtonLongClickOnGoing);
  rButton.setDebounceTicks(BUTTON_DEBOUNCE_TICKS);
  rButton.setClickTicks(BUTTON_CLICK_TICKS);
  rButton.setPressTicks(BUTTON_PRESS_TICKS);

  return true;
}

void DisplayOled::task(void *parameter)
{
  uint8_t flashTimeout = OLED_FLASH_INTERVAL;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  uint8_t bootScreenTimeout = OLED_BOOT_SCREEN_TIME;
  DisplayOled *display = (DisplayOled *)parameter;

  display->system->wireLock();
  display->initDisplay();
  display->system->wireRelease();

  // show boot screen
  while (bootScreenTimeout || display->system->isInitDone() != true)
  {
    vTaskDelayUntil(&xLastWakeTime, OLED_TASK_CYCLE_TIME);
    if (bootScreenTimeout)
      bootScreenTimeout--;
  }

  for (;;)
  {
    vTaskDelayUntil(&xLastWakeTime, 10);

    display->system->wireLock();
    display->update();
    display->system->wireRelease();

    if (!flashTimeout--)
    {
      flashTimeout = OLED_FLASH_INTERVAL;
      display->flashIndicator = !display->flashIndicator;
    }
  }
}

void DisplayOled::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();
  Settings::write(kDisplay, json);
}

void DisplayOled::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kDisplay, &jsonBuffer);

  if (json.success())
  {
  }
}

void DisplayOled::update()
{
  // check global block
  if (!blocked)
  {
    lButton.tick();
    rButton.tick();

    //check oled block
    if (!oledBlocked)
      ui.update();
  }
}

void DisplayOled::handleButtons(ButtonId buttonId, ButtonEvent buttonEvent)
{
  if (ButtonEvent::Click == buttonEvent)
  {
    switch (menuItem)
    {
    case MenuItem::TempShow:
      handleTemperatureNavigation(buttonId);
      break;
    case MenuItem::TempSettingsUpper:
    case MenuItem::TempSettingsLower:
    case MenuItem::TempSettingsType:
    case MenuItem::TempSettingsAlarm:
      if (MenuMode::Show == menuMode)
        handleMenuNavigation((ButtonId::Left == buttonId) ? -1 : 1, MenuItem::TempSettingsUpper, MenuItem::TempSettingsAlarm);
      else if (MenuMode::Edit == menuMode)
        if (menuItem >= MenuItem::TempSettingsType)
          currentData += (ButtonId::Left == buttonId) ? -1 : 1;
        else
          handleTemperatureEdit(buttonId, buttonEvent);
      break;

    case MenuItem::PitmasterSettingsProfile:
    case MenuItem::PitmasterSettingsChannel:
    case MenuItem::PitmasterSettingsTemperature:
    case MenuItem::PitmasterSettingsType:
      if (MenuMode::Show == menuMode)
        handleMenuNavigation((ButtonId::Left == buttonId) ? -1 : 1, MenuItem::PitmasterSettingsProfile, MenuItem::PitmasterSettingsType);
      else if (MenuMode::Edit == menuMode)
      {
        if ((MenuItem::PitmasterSettingsType == menuItem))
          currentData = (((PitmasterType)currentData) != pm_off) ? pm_off : pm_auto;
        else if (MenuItem::PitmasterSettingsTemperature != menuItem)
          currentData += (ButtonId::Left == buttonId) ? -1 : 1;
        else
          handleTemperatureEdit(buttonId, buttonEvent);
      }
      break;

    case MenuItem::SystemSettingsSSID:
    case MenuItem::SystemSettingsIP:
    case MenuItem::SystemSettingsHost:
    case MenuItem::SystemSettingsUnit:
    case MenuItem::SystemSettingsFirmwareVersion:
      if (MenuMode::Show == menuMode)
        handleMenuNavigation((ButtonId::Left == buttonId) ? -1 : 1, MenuItem::SystemSettingsSSID, MenuItem::SystemSettingsFirmwareVersion);
      else if (MenuMode::Edit == menuMode)
      {
        if ((MenuItem::SystemSettingsUnit == menuItem))
          currentData = (((TemperatureUnit)currentData) != Celsius) ? Celsius : Fahrenheit;
      }
      break;

    case MenuItem::MenuTemperature:
    case MenuItem::MenuPitmaster:
    case MenuItem::MenuSystem:
      handleMenuNavigation((ButtonId::Left == buttonId) ? -1 : 1, MenuItem::MenuTemperature, MenuItem::MenuSystem);
      drawMenu();
      break;
    }
  }
  else if (ButtonEvent::LongClickStart == buttonEvent)
  {
    switch (menuItem)
    {
    case MenuItem::TempShow:
      if (ButtonId::Left == buttonId)
      {
        menuItem = MenuItem::TempSettingsUpper;
        menuMode = MenuMode::Show;
        ui.switchToFrame(1u);
        oledBlocked = false;
      }
      else if (ButtonId::Right == buttonId)
      {
        menuItem = MenuItem::MenuTemperature;
        menuMode = MenuMode::Show;
        drawMenu();
      }
      break;
    case MenuItem::TempSettingsUpper:
    case MenuItem::TempSettingsLower:
    case MenuItem::TempSettingsType:
    case MenuItem::TempSettingsAlarm:
      if ((ButtonId::Left == buttonId && MenuMode::Show == menuMode))
      {
        menuItem = MenuItem::TempShow;
        menuMode = MenuMode::Show;
        ui.switchToFrame(0u);
        oledBlocked = false;
      }
      else if ((MenuMode::Edit == menuMode) && (menuItem <= MenuItem::TempSettingsLower))
        handleTemperatureEdit(buttonId, buttonEvent);
      break;

    case MenuItem::PitmasterSettingsProfile:
    case MenuItem::PitmasterSettingsChannel:
    case MenuItem::PitmasterSettingsTemperature:
    case MenuItem::PitmasterSettingsType:
      if ((ButtonId::Left == buttonId && MenuMode::Show == menuMode))
      {
        menuItem = MenuItem::MenuPitmaster;
        menuMode = MenuMode::Show;
        drawMenu();
      }
      else if ((MenuMode::Edit == menuMode) && (MenuItem::PitmasterSettingsTemperature == menuItem))
        handleTemperatureEdit(buttonId, buttonEvent);
      break;

    case MenuItem::SystemSettingsSSID:
    case MenuItem::SystemSettingsIP:
    case MenuItem::SystemSettingsHost:
    case MenuItem::SystemSettingsUnit:
    case MenuItem::SystemSettingsFirmwareVersion:
      if ((ButtonId::Left == buttonId && MenuMode::Show == menuMode))
      {
        menuItem = MenuItem::MenuSystem;
        menuMode = MenuMode::Show;
        drawMenu();
      }
      break;

    case MenuItem::MenuTemperature:
      menuItem = MenuItem::TempShow;
      menuMode = MenuMode::Show;
      ui.switchToFrame(0u);
      oledBlocked = false;
      break;
    case MenuItem::MenuPitmaster:
      if (ButtonId::Left == buttonId)
      {
        menuItem = MenuItem::TempShow;
        menuMode = MenuMode::Show;
        ui.switchToFrame(0u);
        oledBlocked = false;
      }
      else
      {
        menuItem = MenuItem::PitmasterSettingsProfile;
        menuMode = MenuMode::Show;
        ui.switchToFrame(2u);
        oledBlocked = false;
      }
      break;

    case MenuItem::MenuSystem:
      if (ButtonId::Left == buttonId)
      {
        menuItem = MenuItem::TempShow;
        menuMode = MenuMode::Show;
        ui.switchToFrame(0u);
        oledBlocked = false;
      }
      else
      {
        menuItem = MenuItem::SystemSettingsSSID;
        ui.switchToFrame(3u);
        oledBlocked = false;
      }
      break;
    }
  }
  else if (ButtonEvent::LongClickEnd == buttonEvent)
  {
  }
  else if (ButtonEvent::LongClickOnGoing == buttonEvent)
  {
    switch (menuItem)
    {
    case MenuItem::TempSettingsUpper:
    case MenuItem::TempSettingsLower:
    case MenuItem::PitmasterSettingsTemperature:
      if (MenuMode::Edit == menuMode)
        handleTemperatureEdit(buttonId, buttonEvent);
      break;
    }
  }
  else if (ButtonEvent::DoubleClick == buttonEvent)
  {
    switch (menuItem)
    {
    case MenuItem::TempSettingsUpper:
    case MenuItem::TempSettingsLower:
    case MenuItem::TempSettingsType:
    case MenuItem::TempSettingsAlarm:
    case MenuItem::PitmasterSettingsProfile:
    case MenuItem::PitmasterSettingsChannel:
    case MenuItem::PitmasterSettingsTemperature:
    case MenuItem::PitmasterSettingsType:
    case MenuItem::SystemSettingsUnit:
      if ((ButtonId::Right == buttonId && MenuMode::Show == menuMode))
      {
        menuMode = MenuMode::Edit;
      }
      else if ((ButtonId::Right == buttonId && MenuMode::Edit == menuMode))
      {
        menuMode = MenuMode::Set;
      }
      break;
    }
  }
}

void DisplayOled::handleTemperatureEdit(ButtonId buttonId, ButtonEvent buttonEvent)
{
  if (ButtonEvent::LongClickStart == buttonEvent || ButtonEvent::LongClickOnGoing == buttonEvent)
    buttonMupi = 10;
  else
    buttonMupi = 1;

  currentData += (0.1 * (float)buttonMupi) * (ButtonId::Left == buttonId) ? (float)-1 : (float)1;
  if (system->temperatures.getUnit() == Celsius)
  {
    if (currentData > OLIMITMAX)
      currentData = OLIMITMIN;
    else if (currentData < OLIMITMIN)
      currentData = OLIMITMAX;
  }
  else
  {
    if (currentData > OLIMITMAXF)
      currentData = OLIMITMINF;
    else if (currentData < OLIMITMINF)
      currentData = OLIMITMAXF;
  }
}

void DisplayOled::handleMenuNavigation(int8_t add, MenuItem minMenu, MenuItem maxMenu)
{
  if ((int8_t)menuItem + add > (int8_t)maxMenu)
    menuItem = minMenu;
  else if ((int8_t)menuItem + add < (int8_t)minMenu)
    menuItem = maxMenu;
  else
    menuItem = (MenuItem)((int8_t)menuItem + add);
}

void DisplayOled::handleTemperatureNavigation(ButtonId buttonId)
{
  if (ButtonId::Left == buttonId)
  {
    if (currentChannel > 0u)
      currentChannel--;
    else
      currentChannel = system->temperatures.count() - 1u;
  }
  else
  {

    currentChannel++;

    TemperatureBase *nextActiveTemperature = system->temperatures.getNextActive(currentChannel);
    TemperatureBase *firstActiveTemperature = system->temperatures.getNextActive(0u);

    if (nextActiveTemperature != NULL)
      currentChannel = nextActiveTemperature->getGlobalIndex();
    else if (firstActiveTemperature != NULL)
      currentChannel = firstActiveTemperature->getGlobalIndex();
    else if (currentChannel >= system->temperatures.count())
      currentChannel = 0u;
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Frame while system start
void DisplayOled::drawConnect()
{

  oled.clear();
  oled.setColor(WHITE);

  // Draw Logo
  oled.drawXbm(7, 4, nano_width, nano_height, xbmnano);
  oled.display();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Frame while charging
void DisplayOled::drawCharging()
{
  oled.init();
  oled.flipScreenVertically();
  oled.clear();
  oled.setColor(WHITE);

  oled.setTextAlignment(TEXT_ALIGN_CENTER);
  oled.setFont(ArialMT_Plain_10);

  if (gSystem->battery->isCharging())
  {
    oled.fillRect(18,3,2,4);              //Draw battery end button
    oled.drawRect(0,1,17,8);              //Draw Outline Battery
    
    oled.setColor(BLACK);
    oled.fillRect(4,0,8,10);              //Lücke für Pfeil
    oled.setColor(WHITE);
    
    oled.drawXbm(4, 0, 8, 10, xbmcharge); // Ladepfeilspitze
    oled.fillRect(2,3,6,4);               // Ladepfeilstiel
    oled.drawString(64, 30, "WIRD GELADEN...");
  }
  else
  {
    oled.fillRect(18,3,2,4);         //Draw battery end button
    oled.drawRect(0,1,17,8);         //Draw Outline
    oled.fillRect(2,3,MAXBATTERYBAR-1,4);  // Draw Battery Status
    oled.drawString(64, 30, "LADEN BEENDET");
  }

  oled.display();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Frame while Question
void DisplayOled::drawQuestion(int counter)
{
  oled.clear();
  oled.setColor(WHITE);

  oled.setTextAlignment(TEXT_ALIGN_LEFT);
  oled.setFont(ArialMT_Plain_10);

  byte b0 = 1;
  bool b1 = true;

  /*switch (question.typ)
  { // Which Question?

  case CONFIGRESET:
    oled.drawString(3, 3, "Reset Config?");
    break;

  case RESETWIFI:
    oled.drawString(3, 3, "Reset Wifi?");
    break;

  case RESETFW:
    oled.drawString(3, 3, "Reset Firmware?");
    break;

  case IPADRESSE:
    oled.drawString(25, 3, "WLAN-Anmeldung");
    oled.drawString(17, 20, "IP:");
    oled.drawString(33, 20, WiFi.localIP().toString());
    b1 = false;
    b0 = 2;
    break;

  case OTAUPDATE:
    if (gSystem->otaUpdate.get == FIRMWAREVERSION)
      oled.drawString(3, 3, "Update: Erfolgreich!");
    else
      oled.drawString(3, 3, "Update: Fehlgeschlagen!");
    b1 = false;
    b0 = 2;
    break;

  case TUNE:
    if (counter == 0)
    {
      oled.drawString(3, 3, "Autotune: gestartet!");
      oled.setTextAlignment(TEXT_ALIGN_CENTER);
      oled.drawString(64, 20, "PID danach fortsetzen?");
      //oled.drawString(64,18,"fortsetzen?");
      b1 = true;
      b0 = true;
      break;
    }
    else if (counter == 1)
      oled.drawString(3, 3, "Autotune: beendet!");
    else
      oled.drawString(3, 3, "Autotune: abgebrochen!");
    b1 = false;
    b0 = 2;
    break;

  case HARDWAREALARM:
    String text = "ALARM! Kanal ";
    text += String(counter + 1);
    oled.drawString(25, 3, text);
    oled.drawString(40, 18, "Stoppen?");
    b1 = false;
    break;
  }*/

  oled.setFont(ArialMT_Plain_16);
  oled.setTextAlignment(TEXT_ALIGN_LEFT);
  if (b1)
    oled.drawString(10, 40, "NO");
  oled.setTextAlignment(TEXT_ALIGN_RIGHT);
  if (b0 == 1)
    oled.drawString(118, 40, "YES");
  else if (b0 == 2)
    oled.drawString(118, 40, "OK");
  oled.display();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Frame while Update
void DisplayOled::drawUpdate(String txt)
{
  oled.clear();
  oled.setColor(WHITE);

  oled.setTextAlignment(TEXT_ALIGN_LEFT);
  oled.setFont(ArialMT_Plain_10);

  oled.drawString(3, 3, "Update: " + txt);
  oled.setTextAlignment(TEXT_ALIGN_CENTER);
  oled.setFont(ArialMT_Plain_16);
  oled.drawString(64, 28, "Bitte warten!");

  oled.display();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Frame while Menu
void DisplayOled::drawMenu()
{
  oledBlocked = true;
  oled.clear();
  oled.setColor(WHITE);
  oled.setTextAlignment(TEXT_ALIGN_LEFT);
  oled.setFont(ArialMT_Plain_10);

  // Sandwich
  oled.drawLine(3, 3, 13, 3);
  oled.drawLine(3, 7, 13, 7);
  oled.drawLine(3, 11, 13, 11);

  oled.drawXbm(17, 41, arrow_height, arrow_width, xbmarrow2);
  oled.drawXbm(17, 27, arrow_height, arrow_width, xbmarrow1);

  oled.drawString(50, 2, "MENU");
  oled.setFont(ArialMT_Plain_16);

  switch (menuItem)
  {

  case MenuItem::MenuTemperature: // Temperature
    //oled.fillRect(0, 18, 128, 14);
    oled.drawString(30, 27, "Temperatur");
    break;

  case MenuItem::MenuPitmaster: // Pitmaster
    //oled.fillRect(0, 33, 128, 14);
    oled.drawString(30, 27, "Pitmaster");
    break;

  case MenuItem::MenuSystem: // System
    //oled.fillRect(0, 48, 128, 14);
    oled.drawString(30, 27, "System");
    break;
  }

  oled.display();
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// STATUS ROW
void DisplayOled::drawOverlayBar(OLEDDisplay *display, OLEDDisplayUiState *state)
{ 
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);

  if (DisplayBase::debugString.length() != 0u)
  {
    display->drawString(0, 0, DisplayBase::debugString);
    return;
  }

  // PITMASTER STATUS
  boolean showbattery = false;
  Pitmaster *pit = system->pitmasters[0];
  if (pit)
  {
    switch (pit->getType())
    {
    case pm_off:
      if(millis() > OLED_BATTERY_PERCENTAGE_DELAY)
        showbattery = true;
      break;
    case pm_manual:
      display->drawString(33, 0, "M  " + String(pit->getValue(), 0) + "%");
      break;
    case pm_auto:
      /*if (pit->getAssignedProfile()->opl)
        display->drawString(33, 0, "OPL: " + String(opl.temp, 1));
      else*/
      display->drawString(33, 0, "P  " + String(pit->getTargetTemperature(), 1) + " / " + String(pit->getValue(), 0) + "%");
      break;
    }
  }

  // WIFI CONNECTION
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  if (system->wlan.isConnected())
  {
    display->fillRect(116, 9, 2, 1); //Draw ground line
    display->fillRect(120, 9, 2, 1); //Draw ground line
    display->fillRect(124, 9, 2, 1); //Draw ground line

    if (system->wlan.getRssi() > -105)
      display->fillRect(116, 6, 2, 3); //Draw 1 line
    if (system->wlan.getRssi() > -95)
      display->fillRect(120, 4, 2, 5); //Draw 2 line
    if (system->wlan.getRssi() > -80)
      display->fillRect(124, 2, 2, 7); //Draw 3 line
  }
  else if (system->wlan.isAP() && (millis() > OLED_WIFI_AP_DELAY))
  {
    display->drawString(128, 1, "AP");
  }
  else
  {
    display->drawString(128, 0, "");
  }

  // BATTERY STATUS
  if (gSystem->battery)
  {
    int battPixel = 0.5 + ((gSystem->battery->percentage * MAXBATTERYBAR) / 100.0);
    display->setTextAlignment(TEXT_ALIGN_LEFT);

    if (showbattery)
      display->drawString(24, 1, String(system->battery->percentage));
  
    if (flashIndicator && gSystem->battery->percentage < 10)
    {
    } // nothing for flash effect
    else if (gSystem->battery->isCharging())
    {
      display->fillRect(18,5,2,4);              //Draw battery end button
      display->drawRect(0,3,17,8);              //Draw Outline Battery
    
      display->setColor(BLACK);
      display->fillRect(4,2,8,10);              //Lücke für Pfeil
      display->setColor(WHITE);
    
      display->drawXbm(4, 2, 8, 10, xbmcharge); // Ladepfeilspitze
      display->fillRect(2,5,6,4);               // Ladepfeilstiel
    }
    else if (gSystem->battery->isUsbPowered())
    {
      display->drawString(1,1,"USB");
    }
    else
    {
      display->fillRect(18,5,2,4);         //Draw battery end button
      display->drawRect(0,3,17,8);         //Draw Outline
      display->fillRect(2,5,battPixel,4);  // Draw Battery Status
    }
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MAIN TEMPERATURE FRAME
void DisplayOled::drawTemp(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  menuItem = MenuItem::TempShow;
  int match = 0;
  TemperatureBase *temperature = system->temperatures[currentChannel];
  display->drawXbm(x + 19, 18 + y, 20, 36, xbmtemp); // Symbol

  // Show limits in OLED
  if ((temperature->getMaxValue() > temperature->getMinValue()) && temperature->isActive())
  {
    match = map((int)temperature->getValue(), temperature->getMinValue(), temperature->getMaxValue(), 3, 18);
    match = constrain(match, 0, 20);
  }

  display->fillRect(x + 27, y + 43 - match, 4, match); // Current level
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(19 + x, 20 + y, String(currentChannel + 1)); // Channel
  display->drawString(114 + x, 20 + y, temperature->getName());    // Channel Name //utf8ascii()
  display->setFont(ArialMT_Plain_16);
  if ((temperature->getAlarmStatus() != NoAlarm) && (true == flashIndicator))
  {
    if (temperature->getValue() != INACTIVEVALUE)
    {
      if (gSystem->temperatures.getUnit() == Fahrenheit)
        display->drawCircle(100, 41, 2); // Grad-Zeichen
      else
        display->drawCircle(99, 41, 2);                                                                                        // Grad-Zeichen
      display->drawString(114 + x, 36 + y, String(temperature->getValue(), 1) + "  " + (char)gSystem->temperatures.getUnit()); // Channel Temp
    }
    else
      display->drawString(114 + x, 36 + y, "OFF");
  }
  else if (temperature->getAlarmStatus() == NoAlarm)
  {
    if (temperature->getValue() != INACTIVEVALUE)
    {
      if (gSystem->temperatures.getUnit() == Fahrenheit)
        display->drawCircle(100, 41, 2); // Grad-Zeichen
      else
        display->drawCircle(99, 41, 2);                                                                                        // Grad-Zeichen
      display->drawString(114 + x, 36 + y, String(temperature->getValue(), 1) + "  " + (char)gSystem->temperatures.getUnit()); // Channel Temp
    }
    else
      display->drawString(114 + x, 36 + y, "OFF");
  }

  Pitmaster *pitmaster;

  for (int i = 0; i < system->pitmasters.count(); i++)
  {

    pitmaster = system->pitmasters[i];

    if (NULL == pitmaster)
      continue;

    temperature = pitmaster->getAssignedTemperature();

    if (NULL == temperature)
      continue;

    // Show Pitmaster Activity on Icon
    if (pm_auto == pitmaster->getType())
    {
      if (currentChannel == temperature->getGlobalIndex())
      {
        display->setFont(ArialMT_Plain_10);
        if (pitmaster->isAutoTuneRunning())
          display->drawString(44 + x, 31 + y, "A");
        else
          display->drawString(44 + x, 31 + y, "P");

        int _cur = temperature->getValue() * 10;
        int _set = pitmaster->getTargetTemperature() * 10;
        if (_cur > _set)
          display->drawXbm(x + 37, 24 + y, arrow_height, arrow_width, xbmarrow2);
        else if (_cur < _set)
          display->drawXbm(x + 37, 24 + y, arrow_height, arrow_width, xbmarrow1);
        else
          display->drawXbm(x + 37, 24 + y, arrow_width, arrow_height, xbmarrow);
      }
    }
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// TEMPERATURE CONTEXT -Page
void DisplayOled::drawTempSettings(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  if ((true == flashIndicator) || MenuMode::Edit != menuMode)
  {
    display->drawXbm(x + 19, 18 + y, 20, 36, xbmtemp); // Symbol
    //display->fillRect(x + 27, y + 43 - ch[currentChannel].match, 4, ch[currentChannel].match); // Current level
  }
  TemperatureBase *temperature = gSystem->temperatures[currentChannel];

  char unit = (char)gSystem->temperatures.getUnit();

  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(19 + x, 20 + y, String(currentChannel + 1)); // Channel
  //display->drawString(114, 20, menutextde[current_frame]);

  switch (menuItem)
  {

  case MenuItem::TempSettingsUpper: // UPPER LIMIT
    display->drawLine(33 + x, 25 + y, 50, 25);
    display->drawCircle(95, 23, 1); // Grad-Zeichen
    if (MenuMode::Show == menuMode)
    {
      currentData = temperature->getMaxValue();
    }
    else if (MenuMode::Set == menuMode)
    {
      temperature->setMaxValue(currentData);
      menuMode = MenuMode::Show;
      system->temperatures.saveConfig();
    }
    display->drawString(104 + x, 19 + y, String(currentData, 1) + "  " + unit); // Upper Limit
    break;

  case MenuItem::TempSettingsLower: // LOWER LIMIT
    display->drawLine(33 + x, 39 + y, 50, 39);
    display->drawCircle(95, 38, 1); // Grad-Zeichen
    if (MenuMode::Show == menuMode)
    {
      currentData = temperature->getMinValue();
    }
    else if (MenuMode::Set == menuMode)
    {
      temperature->setMinValue(currentData);
      menuMode = MenuMode::Show;
      system->temperatures.saveConfig();
    }
    display->drawString(104 + x, 34 + y, String(currentData, 1) + "  " + unit);
    break;

  case MenuItem::TempSettingsType: // TYP
    display->drawString(114, 20, "TYP");
    if (MenuMode::Show == menuMode)
    {
      currentData = temperature->getType();
    }
    else if (MenuMode::Edit == menuMode)
    {
      if (currentData < 0)
        currentData = temperature->getTypeCount() - 1;
      else if (currentData >= temperature->getTypeCount())
        currentData = 0;
    }
    else if (MenuMode::Set == menuMode)
    {
      temperature->setType((uint8_t)currentData);
      menuMode = MenuMode::Show;
      system->temperatures.saveConfig();
    }
    display->drawString(114 + x, 36 + y, temperature->getTypeName((uint8_t)currentData));
    break;

  case MenuItem::TempSettingsAlarm: // ALARM
    display->drawString(114, 20, "ALARM");
    if (MenuMode::Show == menuMode)
    {
      currentData = temperature->getAlarmSetting();
    }
    else if (MenuMode::Edit == menuMode)
    {
      if (currentData < AlarmMin)
        currentData = AlarmMax - 1;
      else if (currentData >= AlarmMax)
        currentData = AlarmMin;
    }
    else if (MenuMode::Set == menuMode)
    {
      temperature->setAlarmSetting((AlarmSetting)currentData);
      menuMode = MenuMode::Show;
      system->temperatures.saveConfig();
    }
    display->drawString(114 + x, 36 + y, alarmname[(int)currentData]);
    break;
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PITMASTER -Page
void DisplayOled::drawPitmasterSettings(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  if ((true == flashIndicator) || MenuMode::Edit != menuMode)
    display->drawXbm(x + 15, 20 + y, pit_width, pit_height, xbmpit); // Symbol

  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);

  Pitmaster *pm = system->pitmasters[0];
  PitmasterProfile *profile = pm->getAssignedProfile();
  char unit = (char)gSystem->temperatures.getUnit();

  switch (menuItem)
  {

  case MenuItem::PitmasterSettingsProfile: // PID PROFIL
    display->drawString(116, 20, "PITMASTER:");
    if (MenuMode::Show == menuMode)
    {
      currentData = profile->id;
    }
    else if (MenuMode::Edit == menuMode)
    {
      if (currentData < 0)
        currentData = system->getPitmasterProfileCount() - 1;
      else if (currentData >= system->getPitmasterProfileCount())
        currentData = 0;
    }
    else if (MenuMode::Set == menuMode)
    {
      pm->assignProfile(system->getPitmasterProfile(currentData));
      menuMode = MenuMode::Show;
      system->pitmasters.saveConfig();
    }
    display->drawString(116 + x, 36 + y, system->getPitmasterProfile(currentData)->name);
    break;

  case MenuItem::PitmasterSettingsChannel: // PITMASTER CHANNEL
    display->drawString(116, 20, "CHANNEL:");
    if (MenuMode::Show == menuMode)
    {
      currentData = pm->getAssignedTemperature()->getGlobalIndex();
    }
    else if (MenuMode::Edit == menuMode)
    {
      if (currentData < 0)
        currentData = system->temperatures.count() - 1;
      else if (currentData >= system->temperatures.count())
        currentData = 0;
    }
    else if (MenuMode::Set == menuMode)
    {
      pm->assignTemperature(system->temperatures[(uint8_t)currentData]);
      menuMode = MenuMode::Show;
      system->pitmasters.saveConfig();
    }
    display->drawString(116 + x, 36 + y, String(system->temperatures[(uint8_t)currentData]->getGlobalIndex() + 1));
    break;

  case MenuItem::PitmasterSettingsTemperature: // SET TEMPERATUR
    display->drawString(116, 20, "SET:");
    if (MenuMode::Show == menuMode)
    {
      currentData = pm->getTargetTemperature();
    }
    else if (MenuMode::Set == menuMode)
    {
      pm->setTargetTemperature(currentData);
      menuMode = MenuMode::Show;
      system->pitmasters.saveConfig();
    }
    display->drawCircle(107, 40, 1); // Grad-Zeichen
    display->drawString(116 + x, 36 + y, String(currentData, 1) + "  " + unit);
    break;

  case MenuItem::PitmasterSettingsType: // PITMASTER TYP
    display->drawString(116, 20, "ACTIVE:");
    if (MenuMode::Show == menuMode)
    {
      currentData = pm->getType();
    }
    else if (MenuMode::Set == menuMode)
    {
      pm->setType((PitmasterType)currentData);
      menuMode = MenuMode::Show;
      system->pitmasters.saveConfig();
    }

    if ((PitmasterType)currentData == pm_auto)
      display->drawString(116 + x, 36 + y, "AUTO");
    else if ((PitmasterType)currentData == pm_manual)
      display->drawString(116 + x, 36 + y, "MANUAL");
    else if ((PitmasterType)currentData == pm_off)
      display->drawString(116 + x, 36 + y, "OFF");

    break;
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SYSTEM -Page
void DisplayOled::drawSystemSettings(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{

  if ((true == flashIndicator) || MenuMode::Edit != menuMode)
    display->drawXbm(x + 5, 22 + y, sys_width, sys_height, xbmsys);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);

  switch (menuItem)
  {

  case MenuItem::SystemSettingsSSID: // SSID
    display->drawString(120, 20, "SSID:");
    if (system->wlan.isAP())
      display->drawString(120, 36, system->wlan.getAccessPointName());
    else if (system->wlan.isConnected())
      display->drawString(120, 36, WiFi.SSID());
    else
      display->drawString(120, 36, "");
    break;

  case MenuItem::SystemSettingsIP: // IP
    display->drawString(120, 20, "IP:");
    if (system->wlan.isAP())
      display->drawString(120, 36, WiFi.softAPIP().toString());
    else if (system->wlan.isConnected())
      display->drawString(120, 36, WiFi.localIP().toString());
    else
      display->drawString(120, 36, "");
    break;

  case MenuItem::SystemSettingsHost: // HOST
    display->drawString(120, 20, "HOSTNAME:");
    display->drawString(120, 36, system->wlan.getHostName());
    break;

  case MenuItem::SystemSettingsUnit: // UNIT
    display->drawString(120, 20, "UNIT:");
    display->drawCircle(105, 40, 1); // Grad-Zeichen
    if (MenuMode::Show == menuMode)
    {
      currentData = system->temperatures.getUnit();
    }
    else if (MenuMode::Set == menuMode)
    {
      system->temperatures.setUnit((TemperatureUnit)currentData);
      menuMode = MenuMode::Show;
      system->temperatures.saveConfig();
    }
    display->drawString(114 + x, 36 + y, String((char)currentData));
    break;

  case MenuItem::SystemSettingsFirmwareVersion: // FIRMWARE VERSION
    display->drawString(120, 20, "FIRMWARE:");
    display->drawString(114 + x, 36 + y, FIRMWAREVERSION);
    break;
  }
}

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++
// BACK -Page
void drawback(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
  display->drawXbm(x + 5, 22 + y, back_width, back_height, xbmback);
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_16);
  display->drawString(100 + x, 27 + y, "BACK");
}
