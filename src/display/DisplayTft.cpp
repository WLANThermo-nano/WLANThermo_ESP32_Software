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
#include "DisplayTft.h"
#include "Settings.h"
#include "TaskConfig.h"
#include <TFT_eSPI.h>

uint32_t DisplayTft::updateTemperature = 0u;
uint32_t DisplayTft::updatePitmaster = 0u;
SystemBase *DisplayTft::system = gSystem;
uint8_t DisplayTft::serialTimeout = 0u;
boolean DisplayTft::wifiScanInProgress = false;
int8_t DisplayTft::wifiIndex = 0u;
uint8_t DisplayTft::tempPageIndex = 0u;

TFT_eSPI tft = TFT_eSPI();

DisplayTft::DisplayTft()
{
  this->disabled = false;
  this->orientation = DisplayOrientation::_0;
}

void DisplayTft::init()
{
  xTaskCreatePinnedToCore(
      DisplayTft::task,           /* Task function. */
      "DisplayTft::task",         /* String with name of task. */
      10000,                      /* Stack size in bytes. */
      this,                       /* Parameter passed as input of the task */
      TASK_PRIORITY_DISPLAY_TASK, /* Priority of the task. */
      NULL,                       /* Task handle. */
      1);                         /* CPU Core */
}

boolean DisplayTft::initDisplay()
{
  if (this->disabled)
  {
    Serial.printf("DisplayTft::init: display disabled\n");
    return true;
  }

  Serial.printf("TFT_MISO: %i\n", TFT_MISO);
  Serial.printf("TFT_MOSI: %i\n", TFT_MOSI);
  Serial.printf("TFT_SCLK: %i\n", TFT_SCLK);
  Serial.printf("TFT_CS: %i\n", TFT_CS);
  Serial.printf("TFT_DC: %i\n", TFT_DC);

  tft.init();

  tft.setRotation(1);
  tft.fillScreen((0xFFFF));

  tft.setCursor(20, 115, 1);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.setTextSize(3);
  tft.println("Hello WLANThermo");

  return true;
}

void DisplayTft::task(void *parameter)
{
  DisplayTft *display = (DisplayTft *)parameter;

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

void DisplayTft::update()
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
    }
    return;
  }
}

void DisplayTft::calibrate()
{
}