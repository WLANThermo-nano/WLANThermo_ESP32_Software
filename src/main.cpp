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
#include "WServer.h"
#include "DbgPrint.h"
#include "ArduinoLog.h"
#include "LogRingBuffer.h"
#include "TaskConfig.h"
#include "DeviceId.h"

// Forward declaration
void createTasks();

void loggingPrefix(Print *p)
{
  time_t t = now();

  if (year(t) > 2000)
  {
    p->printf("%02d:%02d:%02d: ", hour(t), minute(t), second(t));
  }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SETUP
void setup()
{
  RecoveryMode::run();
  DeviceId::init();

  // Initialize Serial
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Log.begin(LOG_LEVEL_TRACE, &gLogRingBuffer);
  Log.setPrefix(loggingPrefix);
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
    //Serial.printf("MainTask, highWaterMark: %d\n", uxTaskGetStackHighWaterMark(NULL));

    if (gSystem->otaUpdate.isUpdateInProgress())
    {
      // exit task loop for better update performance
      break;
    }

    if (gSystem->otaUpdate.isUpdateInProgress())
    {
      // exit task loop for better update performance
      break;
    }

    // Detect Serial Input
    static char serialbuffer[300];
    if (readline(Serial.read(), serialbuffer, 300) > 0)
    {
      read_serial(serialbuffer);
    }

    // Wait for the next cycle.
    vTaskDelayUntil(&xLastWakeTime, TASK_CYCLE_TIME_MAIN_TASK);
  }

  Serial.println("Delete MainTask task");
  vTaskDelete(NULL);
}

void ConnectTask(void *parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    //Serial.printf("ConnectTask, highWaterMark: %d\n", uxTaskGetStackHighWaterMark(NULL));

    if (gSystem->otaUpdate.isUpdateInProgress())
    {
      // exit task loop for better update performance
      break;
    }

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

  Serial.println("Delete ConnectTask task");
  vTaskDelete(NULL);
}

void createTasks()
{
  xTaskCreatePinnedToCore(
      MainTask,                /* Task function. */
      "MainTask",              /* String with name of task. */
      3000,                    /* Stack size in bytes. */
      NULL,                    /* Parameter passed as input of the task */
      TASK_PRIORITY_MAIN_TASK, /* Priority of the task. */
      NULL,                    /* Task handle. */
      1);                      /* CPU Core */

  xTaskCreatePinnedToCore(
      ConnectTask,                /* Task function. */
      "ConnectTask",              /* String with name of task. */
      3000,                       /* Stack size in bytes. */
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
