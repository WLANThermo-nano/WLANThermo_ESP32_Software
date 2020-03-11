/*************************************************** 
    Copyright (C) 2016  Steffen Ochs, Phantomias2006
    Copyrigth (C) 2019  Martin Koerner

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
#include "RecoveryMode.h"
#include "system/SystemBase.h"
#include "display/DisplayBase.h"
#include "SerialCmd.h"
#include "Server.h"
#include "DbgPrint.h"

// Forward declaration
void createTasks();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SETUP
void setup()
{
  RecoveryMode::run();

  // Initialize Serial
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  gSystem->hwInit();
  gDisplay->init();
  gSystem->loadConfig();
  gSystem->init();
  gSystem->run();

  // Initialize Wifi
  gSystem->wlan.init();

  // Initialize Debug
  dbgPrintInit();

  // Initialize WebServer
  gWebServer.init();

// Initialize OTA
#ifdef OTA
  set_ota();
  ArduinoOTA.begin();
#endif

  // Start all tasks
  createTasks();
}

void MainTask(void *parameter)
{

  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {

    // Wait for the next cycle.
    vTaskDelayUntil(&xLastWakeTime, 100);

    // Detect Serial Input
    static char serialbuffer[300];
    if (readline(Serial.read(), serialbuffer, 300) > 0)
    {
      read_serial(serialbuffer);
    }

    dbgPrintMain();

// Detect OTA
#ifdef OTA
    ArduinoOTA.handle();
#endif
  }
}

void ConnectTask(void *parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    // WiFi - Monitoring
    gSystem->wlan.update();

    if (gSystem->wlan.isConnected())
    {
      gSystem->otaUpdate.update();
      gSystem->mqtt.update();
      gSystem->notification.update();
      gSystem->cloud.update();
    }

    // Wait for the next cycle.
    vTaskDelayUntil(&xLastWakeTime, 1000);
  }
}

void createTasks()
{
  xTaskCreatePinnedToCore(
      MainTask,   /* Task function. */
      "MainTask", /* String with name of task. */
      10000,      /* Stack size in bytes. */
      NULL,       /* Parameter passed as input of the task */
      3,          /* Priority of the task. */
      NULL,       /* Task handle. */
      1);         /* CPU Core */

  xTaskCreatePinnedToCore(
      ConnectTask,   /* Task function. */
      "ConnectTask", /* String with name of task. */
      10000,         /* Stack size in bytes. */
      NULL,          /* Parameter passed as input of the task */
      4,             /* Priority of the task. */
      NULL,          /* Task handle. */
      1);            /* CPU Core */
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LOOP
void loop()
{
  vTaskDelete(NULL);
}
