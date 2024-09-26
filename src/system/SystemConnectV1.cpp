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

#include <SPI.h>
#include <Wire.h>
#include <driver/ledc.h>
#include "SystemConnectV1.h"
#include "temperature/TemperatureMcp3208.h"
#include "temperature/TemperatureMax31855.h"
#include "display/tft/DisplayTft.h"
#include "Constants.h"
#include "DeviceId.h"

// BUZZER
#define BUZZER_IO 2u

// SD CARD
#define CS_SD_CARD 5u

// TFT
#define TFT_RESET_PIN 27u

enum ledcChannels
{
  ledcPitMaster0IO1 = 0,
  ledcPitMaster0IO2 = 1,
  ledcPitMaster1IO1 = 2,
  ledcPitMaster1IO2 = 3,
  ledcBuzzer = 4
};

SystemConnectV1::SystemConnectV1() : SystemBase()
{
}

void SystemConnectV1::hwInit()
{
  pinMode(TFT_RESET_PIN, OUTPUT);
  digitalWrite(TFT_RESET_PIN, HIGH);
  delay(5);
  digitalWrite(TFT_RESET_PIN, LOW);
  delay(20);
  digitalWrite(TFT_RESET_PIN, HIGH);

  // initialize SPI interface
  SPI.begin();

  // initialize I2C interface
  Wire.begin();
}

void SystemConnectV1::init()
{
  deviceName = "connect";
  deviceID = DeviceId::get();
  hardwareVersion = 1u;
  wlan.setHostName(DEFAULT_HOSTNAME + String(serialNumber));

  // add connect feature
  connect = new Connect();
  connect->loadConfig(&temperatures);
  connect->init();

  // load config
  temperatures.loadConfig();

  // initialize buzzer
  buzzer = new Buzzer(BUZZER_IO, ledcBuzzer);

  sdCard = new SdCard(CS_SD_CARD);

  powerSaveModeSupport = true;

  initDone = true;
}
