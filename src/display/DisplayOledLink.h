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
#pragma once

#include <Arduino.h>
#include <SH1106Wire.h>
#include <OLEDDisplayUi.h>
#include <OneButton.h>
#include "system/SystemBase.h"
#include "DisplayBase.h"
#include "DisplayOledTypes.h"

class DisplayOledLink : public DisplayBase
{
public:
  DisplayOledLink();
  void init();
  void update();
  void saveConfig();
  static void drawCharging();

private:
  static SystemBase *system;
  static SH1106Wire oled;
  static OLEDDisplayUi ui;
  static OneButton lButton;
  static OneButton rButton;
  static MenuItem menuItem;
  static MenuMode menuMode;
  static boolean flashIndicator;
  static void task(void *parameter);

  /* Button handling*/
  static void lButtonClick() { handleButtons(ButtonId::Left, ButtonEvent::Click); };
  static void rButtonClick() { handleButtons(ButtonId::Right, ButtonEvent::Click); };
  static void lButtonLongClickStart() { handleButtons(ButtonId::Left, ButtonEvent::LongClickStart); };
  static void lButtonLongClickEnd() { handleButtons(ButtonId::Left, ButtonEvent::LongClickEnd); };
  static void lButtonLongClickOnGoing() { handleButtons(ButtonId::Left, ButtonEvent::LongClickOnGoing); };
  static void rButtonLongClickStart() { handleButtons(ButtonId::Right, ButtonEvent::LongClickStart); };
  static void rButtonLongClickEnd() { handleButtons(ButtonId::Right, ButtonEvent::LongClickEnd); };
  static void rButtonLongClickOnGoing() { handleButtons(ButtonId::Right, ButtonEvent::LongClickOnGoing); };
  static void lButtonDoubleClick() { handleButtons(ButtonId::Left, ButtonEvent::DoubleClick); };
  static void rButtonDoubleClick() { handleButtons(ButtonId::Right, ButtonEvent::DoubleClick); };
  static void handleButtons(ButtonId buttonId, ButtonEvent buttonEvent);
  static void handleTemperatureEdit(ButtonId buttonId, ButtonEvent buttonEvent);
  static void handleMenuNavigation(int8_t add, MenuItem minMenu, MenuItem maxMenu);
  static void handleTemperatureNavigation(ButtonId buttonId);

  void loadConfig();
  boolean initDisplay();

  /* Non UI draws */
  void drawConnect();
  void drawPopUp();
  void drawUpdate(String txt);
  static void drawMenu();

  /* UI draws */
  static FrameCallback frames[];
  static OverlayCallback overlays[];
  static void drawTemp(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
  static void drawTempSettings(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
  static void drawPitmasterSettings(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
  static void drawSystemSettings(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);
  static void drawOverlayBar(OLEDDisplay *display, OLEDDisplayUiState *state);

  boolean handlePopUp();

  static uint8_t currentChannel;
  static float currentData;
  static uint8_t buttonMupi;
  static DisplayPopUpType displayPopUp;
};
