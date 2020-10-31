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
#include "ArduinoLog.h"
#include "LogRingBuffer.h"
#include "TaskConfig.h"

// Forward declaration
void createTasks();

void loggingPrefix(Print *p)
{
  p->printf("%08dms:", millis());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SETUP
void setup()
{
  RecoveryMode::run();

  // Initialize Serial
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Log.begin(LOG_LEVEL_TRACE, &gLogRingBuffer);
  //Log.setPrefix(loggingPrefix);
  Log.notice("Start logging" CR);

  gSystem->hwInit();
  gDisplay->hwInit();
  gDisplay->loadConfig();
  gDisplay->init();
  gSystem->loadConfig();
  gSystem->init();
  gSystem->run();

  // Initialize Wifi
  gSystem->wlan.init();

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

    // Detect Serial Input
    static char serialbuffer[300];
    if (readline(Serial.read(), serialbuffer, 300) > 0)
    {
      read_serial(serialbuffer);
    }

    // Wait for the next cycle.
    vTaskDelayUntil(&xLastWakeTime, TASK_CYCLE_TIME_MAIN_TASK);

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
    vTaskDelayUntil(&xLastWakeTime, TASK_CYCLE_TIME_CONNECT_TASK);
  }
}

void createTasks()
{
  xTaskCreatePinnedToCore(
      MainTask,                /* Task function. */
      "MainTask",              /* String with name of task. */
      10000,                   /* Stack size in bytes. */
      NULL,                    /* Parameter passed as input of the task */
      TASK_PRIORITY_MAIN_TASK, /* Priority of the task. */
      NULL,                    /* Task handle. */
      1);                      /* CPU Core */

  xTaskCreatePinnedToCore(
      ConnectTask,                /* Task function. */
      "ConnectTask",              /* String with name of task. */
      10000,                      /* Stack size in bytes. */
      NULL,                       /* Parameter passed as input of the task */
      TASK_PRIORITY_CONNECT_TASK, /* Priority of the task. */
      NULL,                       /* Task handle. */
      1);                         /* CPU Core */
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// LOOP
void loop()
{
  vTaskDelete(NULL);
}
