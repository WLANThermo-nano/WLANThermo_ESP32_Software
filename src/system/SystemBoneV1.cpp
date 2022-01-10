/*************************************************** 
    Copyright (C) 2021  Martin Koerner

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

#include "SystemBoneV1.h"
#include "Constants.h"

// SD CARD
#define CS_SD_CARD 5u

// BLUETOOTH
#define BLE_UART_TX 12
#define BLE_UART_RX 14
#define BLE_RESET_PIN 4u

SystemBoneV1::SystemBoneV1() : SystemBase()
{
}

void SystemBoneV1::hwInit()
{
}

void SystemBoneV1::init()
{
  deviceName = "bone";
  hardwareVersion = 1u;
  wlan.setHostName(DEFAULT_HOSTNAME + String(serialNumber));

  // add blutetooth feature
  bluetooth = new Bluetooth(BLE_UART_RX, BLE_UART_TX, BLE_RESET_PIN);
  bluetooth->loadConfig(&temperatures);
  bluetooth->init();

  // load config
  temperatures.loadConfig();

  sdCard = new SdCard(CS_SD_CARD);

  initDone = true;
}
