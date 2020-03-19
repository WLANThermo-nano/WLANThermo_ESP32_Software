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
#include "SystemNanoVx.h"
#include "temperature/TemperatureMax11615.h"
#include "display/DisplayOled.h"
#include "Constants.h"

// PITMASTER
#define PITMASTER0IO1 25u   // Fan Pin
#define PITMASTER0IO2 33u   // Data Pin
#define PITMASTERSUPPLY 13u // StepUp Pin

// OLED
#define OLED_RESET_IO 4u

// BUZZER
#define BUZZER_IO 2u

// SD CARD
#define CS_SD_CARD 5u

#define STANDBY_SLEEP_CYCLE_TIME 500000u // 500ms

enum ledcChannels
{
  // Channel 0, Timer0
  ledcPitMaster0IO1 = 0,
  // Channel 1, Timer0
  ledcPitMaster0IO2 = 1,
  // Channel 4, Timer2 --> change to REF_TICK because of automatic light sleep mode
  ledcBuzzer = 4
};

RTC_DATA_ATTR boolean SystemNanoVx::didSleep = false;  // standby ram
RTC_DATA_ATTR boolean SystemNanoVx::didCharge = false; // standby ram

SystemNanoVx::SystemNanoVx() : SystemBase()
{
}

void SystemNanoVx::hwInit()
{
  // only init oled reset pin when coming from cold start
  if (didSleep != true)
  {
    pinMode(OLED_RESET_IO, OUTPUT);
    digitalWrite(OLED_RESET_IO, LOW);
    delay(100);
    digitalWrite(OLED_RESET_IO, HIGH);
    delay(100);
  }

  // initialize battery in hwInit!
  battery = new Battery();
  battery->update();

  pinMode(PITMASTERSUPPLY, OUTPUT);
  digitalWrite(PITMASTERSUPPLY, 0u);

  // handle sleep during battery charge
  if (battery->requestsStandby())
  {
    if (didSleep != true || battery->isCharging() != didCharge)
    {
      Wire.begin();
      Wire.setClock(700000);
      DisplayOled::drawCharging();
      didCharge = battery->isCharging();
    }

    didSleep = true;
    esp_sleep_enable_timer_wakeup(STANDBY_SLEEP_CYCLE_TIME);
    esp_deep_sleep_start();
  }

  didSleep = false;

  Wire.begin();
  Wire.setClock(700000);
}

void SystemNanoVx::init()
{
  deviceName = "nano";
  hardwareVersion = 3u;
  wlan.setHostName(DEFAULT_HOSTNAME + String(serialNumber));

  // initialize temperatures
  this->wireLock();
  temperatures.add(new TemperatureMax11615(0u, &Wire));
  temperatures.add(new TemperatureMax11615(1u, &Wire));
  temperatures.add(new TemperatureMax11615(2u, &Wire));
  temperatures.add(new TemperatureMax11615(3u, &Wire));
  temperatures.add(new TemperatureMax11615(4u, &Wire));
  temperatures.add(new TemperatureMax11615(5u, &Wire));
  temperatures.add(new TemperatureMax11615(6u, &Wire));
  temperatures.add(new TemperatureMax11615(7u, &Wire));
  this->wireRelease();

  // load config
  temperatures.loadConfig();

  // initialize buzzer
  buzzer = new Buzzer(BUZZER_IO, ledcBuzzer);

  // initialize pitmasters
  Pitmaster::setSupplyPin(PITMASTERSUPPLY);
  pitmasters.add(new Pitmaster(PITMASTER0IO1, ledcPitMaster0IO1, PITMASTER0IO2, ledcPitMaster0IO2));

  //        Name,      Nr, Aktor,  Kp,    Ki,  Kd, DCmin, DCmax, JP, SPMIN, SPMAX, LINK, ...
  profile[0] = new PitmasterProfile{"SSR SousVide", 0, 0, 104, 0.2, 0, 0, 100, 100};
  profile[1] = new PitmasterProfile{"TITAN 50x50", 1, 1, 3.8, 0.01, 128, 25, 100, 70};
  profile[2] = new PitmasterProfile{"Kamado 50x50", 2, 1, 7.0, 0.019, 130, 25, 100, 70};

  // default profiles and temperatures, will be overwritten when config exists
  pitmasters[0u]->assignProfile(profile[0]);
  pitmasters[0u]->assignTemperature(temperatures[0]);

  pitmasters.loadConfig();

  sdCard = new SdCard(CS_SD_CARD);

  powerSaveModeSupport = true;
  setPowerSaveMode(true);

  initDone = true;
}
