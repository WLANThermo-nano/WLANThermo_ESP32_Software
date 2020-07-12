/*************************************************** 
    Copyright (C) 2020  Martin Koerner

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
#include "TemperatureMavRadio.h"
#include "BBQ433SnifferV4.h"
#include "ArduinoLog.h"

#define RECEIVER_DETECTION_THRESHOLD 2u
#define QUEUE_MAX_ITEM_NUM 1000u
#define CONNECTION_THRESHOLD 30000u // in ms

xQueueHandle isrQueue = NULL;
uint64_t prevTime = 0u;

uint32_t TemperatureMavRadio::lastUpdateMillis = 0u;
int TemperatureMavRadio::temperatureValue[NUM_OF_MAV_RADIO_TEMPERATURES] = {INACTIVEVALUE, INACTIVEVALUE};

void IRAM_ATTR interruptServiceRoutine()
{
  uint64_t time = esp_timer_get_time();
  uint16_t delta = time - prevTime;
  if (delta >= 170)
    xQueueSendFromISR(isrQueue, &delta, NULL);
  prevTime = time;
}

TemperatureMavRadio::TemperatureMavRadio()
{
}

TemperatureMavRadio::TemperatureMavRadio(uint8_t index) : TemperatureBase()
{
  this->address = TEMPERATURE_ADDRESS_MAVERICK_RADIO;
  this->localIndex = index;
  this->fixedSensor = true;
  this->type = SensorType::MaverickRadio;
}

void TemperatureMavRadio::update()
{
  if (0u == localIndex)
  {
    uint16_t delta;
    uint32_t queueItemsCount = 0u;

    while (xQueueReceive(isrQueue, &delta, 0))
    {
      queueItemsCount++;

      if (bbq433CheckData(delta))
      {
        xQueueReset(isrQueue);
        temperatureValue[0] = bbq433GetTemperature(0);
        temperatureValue[1] = bbq433GetTemperature(1);
        lastUpdateMillis = millis();
        //Serial.printf("Temperature 0: %d\n", temperatureValue[0]);
        //Serial.printf("Temperature 1: %d\n\n", temperatureValue[1]);
        break;
      }
    }

    //Serial.printf("queueItemsCount: %d\n", queueItemsCount);
  }
}

void TemperatureMavRadio::refresh()
{
  if (localIndex < NUM_OF_MAV_RADIO_TEMPERATURES)
  {
    if (((millis() - lastUpdateMillis) > CONNECTION_THRESHOLD) || (0u == lastUpdateMillis))
    {
      this->currentValue = (float)INACTIVEVALUE;
      this->connected = false;
    }
    else
    {
      this->currentValue = (float)temperatureValue[localIndex];
      this->connected = true;
    }
  }
}

boolean TemperatureMavRadio::initReceiver(uint8_t rxPin)
{
  boolean recAvailable = false;
  isrQueue = xQueueCreate(QUEUE_MAX_ITEM_NUM, sizeof(uint16_t));

  pinMode(rxPin, INPUT);

  attachInterrupt(rxPin, interruptServiceRoutine, CHANGE);

  delay(50);

  uint16_t delta;
  uint32_t queueItemsCount = 0u;

  while (xQueueReceive(isrQueue, &delta, 0))
  {
    queueItemsCount++;
  }

  if (queueItemsCount >= RECEIVER_DETECTION_THRESHOLD)
  {
    recAvailable = true;
    Log.notice("433 Mhz receiver has been detected" CR);
  }
  else
  {
    detachInterrupt(rxPin);
    vQueueDelete(isrQueue);
  }

  return recAvailable;
}
