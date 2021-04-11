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

    SOURCE: https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/adc.html
    
****************************************************/
#include "Battery.h"
#include "system/SystemBase.h"
#include "display/DisplayBase.h"
#include "Settings.h"
#include <esp_adc_cal.h>
#include "ArduinoLog.h"

#define BATTERY_ADC_IO 39u
#define BATTERTY_CHARGE_IO 35u
#define BATTERY_USB_IO 34u
#define MEDIAN_SIZE 30u
#define PERCENTAGE_UPDATE_CYCLE 30u // 30 seconds
#define BATTMIN 3500        // MINIMUM BATTERY VOLTAGE in mV
#define BATTMAX 4180        // MAXIMUM BATTERY VOLTAGE in mV
#define BATTDIV 5.7F        // VOLTAGE DIVIDER
#define CORRECTIONTIME 60000
#define BATTERYSTARTUP 20000
#define SWITCHED_OFF_THRESHOLD 100u
#define STARTUP_COMPENSATION 15u

esp_adc_cal_characteristics_t *adc_chars = new esp_adc_cal_characteristics_t;

Battery::Battery()
{
  pinMode(BATTERTY_CHARGE_IO, INPUT);
  pinMode(BATTERY_USB_IO, INPUT_PULLDOWN);              // hat kein Pulldown

  // Calibration function
  analogSetPinAttenuation(BATTERY_ADC_IO, ADC_0db);
  analogSetAttenuation(ADC_0db);
  analogSetWidth(10u);

  this->min = BATTMIN;
  this->max = BATTMAX;
  this->setreference = 0;
  this->percentage = 0;
  this->standbyRequest = false;
  this->nobattery = false;
  this->correction = 0;
  adcVoltMedian = new MedianFilter<uint16_t>(MEDIAN_SIZE);
  loadConfig();

  updatePowerMode();
}

void Battery::update()
{
  updatePowerMode();
  setReference();
  updatePowerPercentage();
}

void Battery::updatePowerPercentage()
{

  static uint8_t updateCycle = 0u;

  // update percentage every 30 seconds
  if(updateCycle > 0u)
  {
    updateCycle--;
    return;
  }
  else if (millis() > BATTERYSTARTUP)
  {
    updateCycle = PERCENTAGE_UPDATE_CYCLE;
  }
  //Serial.println("Battery::updatePowerPercentage");

  // Calculate battery percentage - polynom
  float rVol = this->voltage/1000.0;
  uint32_t percraw = (1664.4 - 197.3*rVol)*rVol - 3408.0;
  percraw = constrain(percraw, 0, 100);

  // Mode Correction
  switch (this->powerMode)
  {

  case PowerMode::USB:
    if (millis() > 10000)
    {
      // immer 100% bei Betrieb über USB-Quelle und Akku vollgeladen
      this->percentage = 100;
    }
    break;

  case PowerMode::Charging:
    // Anzeige während des Ladens auf 99 % beschränken
    this->percentage = constrain(percraw, 0, 99);
    break;

  case PowerMode::NoBattery:
    this->percentage = 100;
    break;

  case PowerMode::Battery:
    // Nach vollständiger Ladung 
    if (this->setreference > 0)
    {
      this->percentage = 100;
      if ((millis() - this->correction) > CORRECTIONTIME)
      {
        this->setreference -= CORRECTIONTIME / 1000;
        this->setreference = constrain(this->setreference, 0, 180);
        saveConfig();
        this->correction = millis();
      }
    }
    // Freier Betrieb
    else {
      if (millis() < BATTERYSTARTUP)
      {
        this->percentage = percraw;
      }
      else 
      {
        int FF = 2;
        this->percentage = ceil(((this->percentage * FF) + percraw) / (FF +1.0));
      }
    }
    break;

  }
}

void Battery::updatePowerMode()
{
  // Supply & Charging Detection
  this->usbPowerEnabled = digitalRead(BATTERY_USB_IO);
  this->chargeEnabled = (true == this->usbPowerEnabled) ? !digitalRead(BATTERTY_CHARGE_IO) : false;

  // Battery Digital Value Measurement
  this->adcVoltValue = analogReadMilliVolts(BATTERY_ADC_IO);
  
  // Switch Position Detection
  this->switchedOff = (SWITCHED_OFF_THRESHOLD > this->adcVoltValue) ? true : false;

  // Standby Detection
  if (this->switchedOff == true)
  {
    // this->powerMode = PowerMode::Standby;  stimmt nicht ganz kann auch Charging sein
    this->standbyRequest = true;
    return;
  }

  this->standbyRequest = false;

  // Filter Value
  if (millis() < BATTERYSTARTUP)
  {
    // Beim Start anheben um Spannungsverlust auszugleichen
    this->adcVoltFilteredValue = this->adcVoltValue + STARTUP_COMPENSATION;
  }
  else
  {
    this->adcVoltFilteredValue = adcVoltMedian->AddValue(this->adcVoltValue);
  }
  
  // Correction Voltage Divider
  this->adcvoltage = this->adcVoltValue * BATTDIV;
  this->voltage = this->adcVoltFilteredValue * BATTDIV;

  // Power Mode Selection
  if (this->nobattery == true)
  {
    this->powerMode = PowerMode::NoBattery;     // No Battery
    this->chargeEnabled = false;
  }
  else if ((this->voltage < this->min) && (this->usbPowerEnabled == false))
  {
    this->powerMode = PowerMode::Protection;    // Protection
    Log.notice("Battery: Protection" CR);
  }
  else if ((this->usbPowerEnabled == true) && (this->chargeEnabled == true))
  {
    this->powerMode = PowerMode::Charging;      // Charging (Switch ON or OFF)
  }
  else if ((this->usbPowerEnabled == false) && (this->chargeEnabled == false))
  {
    this->powerMode = PowerMode::Battery;       // Discharging
  }
  else if ((this->switchedOff == false) && (this->usbPowerEnabled == true) && (this->chargeEnabled == false))
  {
    this->powerMode = PowerMode::USB;           // Charging Completed
  }
  else if ((this->switchedOff == true) && (this->usbPowerEnabled == true) && (this->chargeEnabled == false))
  {
    this->powerMode = PowerMode::Standby;       // Charging Completed, hier nur zur Vollstaendigkeit
  }

}

void Battery::setReference()
{
  switch (this->powerMode)
  {

  case PowerMode::Charging:
    if (this->setreference != -1 && this->setreference < 180)
    {
      this->setreference = -1;    // Zurücksetzen
      saveConfig();
    }
    break;

  case PowerMode::USB:
  case PowerMode::Standby:
    if (this->setreference == -1)
    {
      this->setreference = 180;       // Referenzzeit setzen
      if (this->voltage > 4100)       // setze Referenz wenn Akku wirklich geladen
      {
        this->max = constrain(this->voltage, 4100, 4180);
      }
      saveConfig();
    }
    break;

  }
}

void Battery::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();
  json["batmax"]  = this->max;
  json["batmin"]  = this->min;
  json["batfull"] = this->setreference;
  Settings::write(kBattery, json);
}

void Battery::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kBattery, &jsonBuffer);

  if (json.success())
  {
    if (json.containsKey("batmax"))
      this->max = json["batmax"];
    if (json.containsKey("batmin"))
      this->min = json["batmin"];
    if (json.containsKey("batfull"))
      this->setreference = json["batfull"];    
  }
}