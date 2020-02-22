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
#pragma once

#include "Arduino.h"
#include "MedianFilterLib.h"

#define INACTIVEVALUE 999
#define TEMPERATURE_TYPE_NOT_CHANGEABLE 0xFFu
#define NUM_OF_TYPES 12u

typedef void (*TemperatureCallback_t)(class TemperatureBase*, boolean, void*); 

enum AlarmSetting
{
  AlarmMin = 0u,
  AlarmOff = AlarmMin,
  AlarmViaPush = 1u,
  AlarmViaSummer = 2u,
  AlarmAll = 3u,
  AlarmMax = 4u,
};

enum AlarmStatus
{
  NoAlarm = 0u,
  MinAlarm = 1u,
  MaxAlarm = 2u
};

enum TemperatureUnit
{
  Fahrenheit = 'F',
  Celsius = 'C',
};

class TemperatureBase
{
  public:
    TemperatureBase();
    ~TemperatureBase();
    void loadDefaultValues();
    float getValue();
    float GetMedianValue();
    float getMinValue();
    float getMaxValue();
    String getName();
    String getColor();
    AlarmSetting getAlarmSetting();
    uint8_t getType();
    static uint8_t getTypeCount();
    String getTypeName();
    static String getTypeName(uint8_t index);
    uint8_t getGlobalIndex();
    virtual void setType(uint8_t type);
    void setMinValue(float value);
    void setMaxValue(float value);
    void setName(const char* name);
    void setColor(const char* color);
    void setAlarmSetting(AlarmSetting alarmSetting);
    void setUnit(TemperatureUnit unit);
    uint8_t getNotificationCounter();
    void setNotificationCounter(uint8_t notificationCounter);
    void updateNotificationCounter();
    void registerCallback(TemperatureCallback_t callback, void *userData);
    void unregisterCallback();
    void handleCallbacks();
    AlarmStatus getAlarmStatus();
    boolean isActive();
    void virtual update();
  protected:
    uint8_t localIndex;
    uint8_t globalIndex;
    float currentValue;
    MedianFilter<float> *medianValue;
    float minValue;
    float maxValue;
    uint8_t type;
    String name;
    String color;
    AlarmSetting alarmSetting;
  private:
    TemperatureUnit currentUnit;
    uint8_t notificationCounter;
    static const char* typeNames[NUM_OF_TYPES];
    TemperatureCallback_t registeredCb;
    boolean settingsChanged;
    void  *registeredCbUserData;
    AlarmStatus cbAlarmStatus;
    float cbCurrentValue;
    static uint8_t globalIndexTracker;
    float getUnitValue(float value);
    float setUnitValue(float value);
};