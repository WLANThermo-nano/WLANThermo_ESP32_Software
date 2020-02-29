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

#include "TemperatureBase.h"

#define LOWEST_VALUE -31
#define HIGHEST_VALUE 999
#define DEFAULT_MIN_VALUE 10.0
#define DEFAULT_MAX_VALUE 35.0
#define MAX_COLORS 12u
#define NUM_OF_TYPES 12
#define MEDIAN_SIZE 10u
#define DEFAULT_CHANNEL_NAME "Kanal "

const static String colors[MAX_COLORS] = {"#0C4C88", "#22B14C", "#EF562D", "#FFC100", "#A349A4", "#804000", "#5587A2", "#5C7148", "#5C7148", "#5C7148", "#5C7148", "#5C7148"};
const char* TemperatureBase::typeNames[NUM_OF_TYPES] = {"1000K/Maverick","Fantast-Neu","Fantast","100K/iGrill2","ET-73",
                                      "Perfektion","50K","Inkbird","100K6A1B","Weber_6743",
                                      "Santos","5K3A1B"};
uint8_t TemperatureBase::globalIndexTracker = 0u;

TemperatureBase::TemperatureBase()
{
  this->registeredCb = NULL;
  this->globalIndex = this->globalIndexTracker++;
  this->medianValue = new MedianFilter<float>(MEDIAN_SIZE);
  this->loadDefaultValues();
  this->settingsChanged = false;
  this->cbCurrentValue = INACTIVEVALUE;
  this->cbAlarmStatus = NoAlarm;
}

TemperatureBase::~TemperatureBase()
{
  this->globalIndexTracker--;
}

void TemperatureBase::loadDefaultValues()
{
  this->currentUnit = Celsius;
  this->currentValue = INACTIVEVALUE;
  this->minValue = DEFAULT_MIN_VALUE;
  this->maxValue = DEFAULT_MAX_VALUE;
  this->name = DEFAULT_CHANNEL_NAME + String(this->globalIndex + 1u);
  this->type = 0u;
  this->alarmSetting = AlarmOff;
  this->notificationCounter = 1u;
  if (this->globalIndex < MAX_COLORS)
    this->color = colors[this->globalIndex];
  
  settingsChanged = true;
}

void TemperatureBase::registerCallback(TemperatureCallback_t callback, void *userData)
{
   this->registeredCb = callback;
   this->registeredCbUserData = userData;
}

void TemperatureBase::unregisterCallback()
{
   this->registeredCb = NULL;
}

void TemperatureBase::handleCallbacks()
{
  AlarmStatus newAlarmStatus = getAlarmStatus();

  if((this->registeredCb != NULL))
  {
    if((true == settingsChanged) || (cbAlarmStatus != newAlarmStatus) || (cbCurrentValue != currentValue))
    {
      this->registeredCb(this, settingsChanged, this->registeredCbUserData);
      cbAlarmStatus = newAlarmStatus;
      settingsChanged = false;
      cbCurrentValue = getValue();
    }
  }
}

float TemperatureBase::getValue()
{
  return (this->currentValue == INACTIVEVALUE) ? INACTIVEVALUE : getUnitValue(this->currentValue);
}

float TemperatureBase::GetMedianValue()
{
  return getUnitValue(medianValue->AddValue(this->currentValue));
}

float TemperatureBase::getMinValue()
{
  return getUnitValue(this->minValue);
}

float TemperatureBase::getMaxValue()
{
  return getUnitValue(this->maxValue);
}

String TemperatureBase::getName()
{
  return this->name;
}

String TemperatureBase::getColor()
{
  return this->color;
}

AlarmSetting TemperatureBase::getAlarmSetting()
{
  return this->alarmSetting;
}

uint8_t TemperatureBase::getType()
{
  return this->type;
}

uint8_t TemperatureBase::getTypeCount()
{
  return NUM_OF_TYPES;
}

String TemperatureBase::getTypeName()
{
  return getTypeName(this->type);
}

String TemperatureBase::getTypeName(uint8_t index)
{
  return (index < NUM_OF_TYPES) ? typeNames[index] : "";
}

uint8_t TemperatureBase::getGlobalIndex()
{
  return this->globalIndex;
}

void TemperatureBase::setType(uint8_t type)
{
  this->type = type;
}

void TemperatureBase::setMinValue(float value)
{
  if (value > LOWEST_VALUE && value < HIGHEST_VALUE)
  {
    this->minValue = setUnitValue(value);
    settingsChanged = true;
  }
}

void TemperatureBase::setMaxValue(float value)
{
  if (value > LOWEST_VALUE && value < HIGHEST_VALUE)
  {
    this->maxValue = setUnitValue(value);
    settingsChanged = true;
  }
}

void TemperatureBase::setName(const char *name)
{
  this->name = name;
  settingsChanged = true;
}

void TemperatureBase::setColor(const char *color)
{
  this->color = color;
  settingsChanged = true;
}

void TemperatureBase::setAlarmSetting(AlarmSetting alarmSetting)
{
  this->alarmSetting = alarmSetting;
  settingsChanged = true;
}

void TemperatureBase::setUnit(TemperatureUnit unit)
{
  this->currentUnit = unit;
  settingsChanged = true;
}

uint8_t TemperatureBase::getNotificationCounter()
{
  return this->notificationCounter;
}


void TemperatureBase::setNotificationCounter(uint8_t notificationCounter)
{
  this->notificationCounter = notificationCounter;
}

void TemperatureBase::updateNotificationCounter()
{
  if (this->notificationCounter > 0u)
    this->notificationCounter--;
}

AlarmStatus TemperatureBase::getAlarmStatus()
{
  AlarmStatus status = NoAlarm;

  if (INACTIVEVALUE == this->currentValue)
    status = NoAlarm;
  else if (this->currentValue <= this->minValue)
    status = MinAlarm;
  else if (this->currentValue >= this->maxValue)
    status = MaxAlarm;

  return status;
}

boolean TemperatureBase::isActive()
{
  return (INACTIVEVALUE != this->currentValue);
}

void TemperatureBase::update()
{
}

float TemperatureBase::getUnitValue(float value)
{
  float convertedValue = value;

  if (this->currentUnit == Fahrenheit)
  {
    convertedValue = ((value * (9 / 5)) + 32);
  }

  return convertedValue;
}

float TemperatureBase::setUnitValue(float value)
{
  float convertedValue = value;

  if (this->currentUnit == Fahrenheit)
  {
    convertedValue = (value - 32) * (5 / 9);
  }

  return convertedValue;
}