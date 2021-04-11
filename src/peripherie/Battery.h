/*************************************************** 
    Copyright (C) 2016  Steffen Ochs
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
#pragma once

#include "Arduino.h"
#include "MedianFilterLib.h"

enum class PowerMode
{
  Standby,            // Standby, USB Operation, no charging
  USB,                // USB Operation, no charging
  Charging,           // USB Operation, charging
  Battery,            // Battery Operation
  Protection,         // Battery Protection Operation
  NoBattery,          // No Battery Connected
};

class Battery
{
public:
  Battery();
  void update();
  boolean isUsbPowered() {return this->usbPowerEnabled;};
  boolean isCharging(){return this->chargeEnabled;};
  boolean requestsStandby(){return this->standbyRequest;};
  int getPowerModeInt(){return (int)this->powerMode;};
  void saveConfig();

  int voltage;      // FILTERED VOLTAGE
  int adcvoltage;   // CURRENT VOLTAGE
  int percentage;   // BATTERY CHARGE STATE in %
  int setreference; // LOAD COMPLETE SAVE VOLTAGE
  int max;          // MAX VOLTAGE
  int min;          // MIN VOLTAGE
  boolean nobattery;

  int correction = 0;
  int sim;    // SIMULATION VOLTAGE
  byte simc;  // SIMULATION COUNTER
  

private:
  void updatePowerMode();
  void updatePowerPercentage();
  void setReference();
  void loadConfig();
  PowerMode powerMode;
  boolean usbPowerEnabled;
  boolean chargeEnabled;
  boolean switchedOff;
  uint16_t adcVoltValue;
  uint32_t adcVoltFilteredValue;
  boolean standbyRequest;
  MedianFilter<uint16_t> *adcVoltMedian;
  
};
