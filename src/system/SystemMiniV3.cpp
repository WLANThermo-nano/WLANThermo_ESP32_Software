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
#include "SystemMiniV3.h"
#include "temperature/TemperatureMcp3208.h"
#include "temperature/TemperatureMax31855.h"
#include "display/DisplayNextion.h"
#include "Constants.h"

// TEMPERATURES
#define CS_MCP3208 0u
#define CS_MAX31855_N1 15u

// PITMASTER
#define PITMASTER0IO1 25u   // Fan Pin
#define PITMASTER0IO2 33u   // Data Pin
#define PITMASTER1IO1 26u   // Fan Pin
#define PITMASTER1IO2 27u   // Data Pin
#define PITMASTERSUPPLY 13u // StepUp Pin

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
  ledcPitMaster0IO1 = 0,
  ledcPitMaster0IO2 = 1,
  ledcPitMaster1IO1 = 2,
  ledcPitMaster1IO2 = 3,
  ledcBuzzer = 4
};

SystemMiniV3::SystemMiniV3() : SystemBase()
{
}

void SystemMiniV3::hwInit()
{
  // initialize battery in hwInit!
  battery = new Battery();
  battery->update();

  pinMode(PITMASTERSUPPLY, OUTPUT);
  digitalWrite(PITMASTERSUPPLY, 0u);
}

void SystemMiniV3::init()
{
  deviceName = "mini";
  hardwareVersion = 3u;
  wlan.setHostName(DEFAULT_HOSTNAME + String(serialNumber));

  // configure PIN mode
  pinMode(CS_MCP3208, OUTPUT);
  pinMode(CS_MAX31855_N1, OUTPUT);

  // set initial PIN state
  digitalWrite(CS_MCP3208, HIGH);
  digitalWrite(CS_MAX31855_N1, HIGH);

  // initialize SPI interface
  SPI.begin();

  // initialize I2C interface
  Wire.begin();

  // initialize temperatures
  temperatures.add(new TemperatureMcp3208(0u, CS_MCP3208));
  temperatures.add(new TemperatureMcp3208(1u, CS_MCP3208));
  temperatures.add(new TemperatureMcp3208(2u, CS_MCP3208));
  temperatures.add(new TemperatureMcp3208(3u, CS_MCP3208));
  temperatures.add(new TemperatureMcp3208(4u, CS_MCP3208));
  temperatures.add(new TemperatureMcp3208(5u, CS_MCP3208));
  temperatures.add(new TemperatureMcp3208(6u, CS_MCP3208));
  temperatures.add(new TemperatureMcp3208(7u, CS_MCP3208));

  if (false == disableTypeK)
  {
    //check if thermocouple is built in
    TemperatureMax31855 *checkThermocouple = new TemperatureMax31855(0u, CS_MAX31855_N1);
    if (checkThermocouple->isBuiltIn())
    {
      temperatures.add(checkThermocouple);
    }
    else
    {
      delete (checkThermocouple);
    }
  }

  // add blutetooth feature
  bluetooth = new Bluetooth(BLE_UART_RX, BLE_UART_TX, BLE_RESET_PIN);
  bluetooth->loadConfig(&temperatures);
  bluetooth->init();

  // load config
  temperatures.loadConfig();

  // initialize buzzer
  buzzer = new Buzzer(BUZZER_IO, ledcBuzzer);

  // initialize pitmasters
  Pitmaster::setSupplyPin(PITMASTERSUPPLY);
  pitmasters.add(new Pitmaster(PITMASTER0IO1, ledcPitMaster0IO1, PITMASTER0IO2, ledcPitMaster0IO2));
  pitmasters.add(new Pitmaster(PITMASTER1IO1, ledcPitMaster1IO1, PITMASTER1IO2, ledcPitMaster1IO2));

  //        Name,      Nr, Aktor,  Kp,    Ki,  Kd, DCmin, DCmax, JP, SPMIN, SPMAX, LINK, ...
  profile[pitmasterProfileCount++] = new PitmasterProfile{"SSR SousVide", 0, 0, 104, 0.2, 0, 0, 100, 100};
  profile[pitmasterProfileCount++] = new PitmasterProfile{"TITAN 50x50", 1, 1, 3.8, 0.01, 128, 25, 100, 70};
  profile[pitmasterProfileCount++] = new PitmasterProfile{"Servo MG995", 2, 2, 104, 0.2, 0, 0, 100, 100, 25, 75};
  profile[pitmasterProfileCount++] = new PitmasterProfile{"Damper", 3, 3, 3.8, 0.01, 128, 25, 100, 70, 25, 75, 0};

  // Add Damper support
  damperSupport = true;

  // default profiles and temperatures, will be overwritten when config exists
  pitmasters[0u]->assignProfile(profile[0]);
  pitmasters[0u]->assignTemperature(temperatures[0]);
  pitmasters[1u]->assignProfile(profile[1]);
  pitmasters[1u]->assignTemperature(temperatures[1]);

  pitmasters.loadConfig();

  sdCard = new SdCard(CS_SD_CARD);

  powerSaveModeSupport = true;
  setPowerSaveMode(true);

  initDone = true;
}
