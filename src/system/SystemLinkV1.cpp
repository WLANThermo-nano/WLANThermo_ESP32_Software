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

#include <SPI.h>
#include <Wire.h>
#include <driver/ledc.h>
#include "SystemLinkV1.h"
#include "temperature/TemperatureMax11613.h"
#include "display/DisplayOledLink.h"
#include "Constants.h"
#include "DeviceId.h"

// PITMASTER
#define PITMASTER0IO1 25u   // Fan Pin
#define PITMASTER0IO2 33u   // Data Pin
#define PITMASTERSUPPLY 13u // StepUp Pin

// OLED
//#define OLED_RESET_IO 4u

// BUZZER
#define BUZZER_IO 2u

// SD CARD
#define CS_SD_CARD 5u

// BLUETOOTH
#define BLE_UART_TX 12
#define BLE_UART_RX 14
#define BLE_RESET_PIN 4u

enum ledcChannels
{
  // Channel 0, Timer0
  ledcPitMaster0IO1 = 0,
  // Channel 1, Timer0
  ledcPitMaster0IO2 = 1,
  // Channel 4, Timer2 --> change to REF_TICK because of automatic light sleep mode
  ledcBuzzer = 4
};

SystemLinkV1::SystemLinkV1() : SystemBase()
{
}

void SystemLinkV1::hwInit()
{
  pinMode(PITMASTERSUPPLY, OUTPUT);
  digitalWrite(PITMASTERSUPPLY, 0u);

  Wire.begin();
  Wire.setClock(700000);
}

void SystemLinkV1::init()
{
  deviceName = "link";
  deviceID = DeviceId::get();
  hardwareVersion = 1u;
  wlan.setHostName(DEFAULT_HOSTNAME + String(serialNumber));

  // initialize temperatures
  this->wireLock();
  temperatures.add(new TemperatureMax11613(0u, &Wire));
  temperatures.add(new TemperatureMax11613(1u, &Wire));
  temperatures.add(new TemperatureMax11613(2u, &Wire));
  this->wireRelease();

  // add blutetooth feature
  bluetooth = new Bluetooth(BLE_UART_RX, BLE_UART_TX, BLE_RESET_PIN);
  bluetooth->loadConfig(&temperatures);
  bluetooth->init();

  // load config
  temperatures.loadConfig();

  // initialize buzzer
  buzzer = new Buzzer(BUZZER_IO, ledcBuzzer);
/*
  // initialize pitmasters
  Pitmaster::setSupplyPin(PITMASTERSUPPLY);
  pitmasters.add(new Pitmaster(PITMASTER0IO1, ledcPitMaster0IO1, PITMASTER0IO2, ledcPitMaster0IO2));

  //        Name,      Nr, Aktor,  Kp,    Ki,  Kd, DCmin, DCmax, JP, SPMIN, SPMAX, LINK, ...
  profile[pitmasterProfileCount++] = new PitmasterProfile{"SSR SousVide", 0, 0, 104, 0.2, 0, 0, 100, 100};
  profile[pitmasterProfileCount++] = new PitmasterProfile{"TITAN 50x50", 1, 1, 3.8, 0.01, 128, 25, 100, 70};
  profile[pitmasterProfileCount++] = new PitmasterProfile{"Kamado 50x50", 2, 1, 7.0, 0.019, 130, 25, 100, 70};
  profile[pitmasterProfileCount++] = new PitmasterProfile{"Custom", 3, 1, 7.0, 0.2, 0, 0, 100, 100, 0, 100};

  // default profiles and temperatures, will be overwritten when config exists
  pitmasters[0u]->assignProfile(profile[0]);
  pitmasters[0u]->assignTemperature(temperatures[0]);

  pitmasters.loadConfig();
*/

  // initialize PBGuard (temporary)
  pinMode(26u, OUTPUT);
  digitalWrite(26u, HIGH);

  sdCard = new SdCard(CS_SD_CARD);

  //pbGuard = new PbGuard();

  initDone = true;
}
