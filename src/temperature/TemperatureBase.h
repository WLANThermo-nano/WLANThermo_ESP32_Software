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
#include "MedianFilterFloat.h"
#include "TemperatureSensors.h"

#define INACTIVEVALUE 999

#define TEMPERATURE_ADDRESS_INTERNAL "FF:FF:FF:00:00:00"
#define TEMPERATURE_ADDRESS_TYPE_K "FF:FF:FF:00:01:00"
#define TEMPERATURE_ADDRESS_MAVERICK_RADIO "FF:FF:FF:00:02:00"

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

typedef float (*TemperatureCalculation_t)(uint16_t, SensorType);

class TemperatureBase
{
public:
  TemperatureBase();
  ~TemperatureBase();
  void loadDefaultValues(uint8_t index);
  void loadConfig(TemperatureUnit unit);
  float getValue();
  float getPreValue();
  int8_t getGradient();
  float getMinValue();
  float getMaxValue();
  String getName();
  String getAddress();
  String getColor();
  uint8_t getLocalIndex() { return localIndex; };
  AlarmSetting getAlarmSetting();
  uint8_t getType();
  static uint8_t getTypeCount();
  String getTypeName();
  TemperatureUnit getUnit();
  static String getTypeName(uint8_t index);
  static boolean isTypeFixed(uint8_t index);
  boolean isFixedSensor() { return this->fixedSensor; }
  boolean isConnected() { return this->connected; }
  virtual void setType(uint8_t type);
  void setMinValue(float value);
  void setMaxValue(float value);
  void setName(const char *address);
  void setAddress(const char *name);
  void setColor(const char *color);
  void setColor(uint32_t color);
  void setAlarmSetting(AlarmSetting alarmSetting);
  void setUnit(TemperatureUnit unit);
  uint8_t getNotificationCounter();
  void setNotificationCounter(uint8_t notificationCounter);
  void acknowledgeAlarm() { acknowledgedAlarm = true; };
  boolean isAlarmAcknowledged() { return acknowledgedAlarm; };
  void updateNotificationCounter();
  boolean checkNewValue();
  boolean checkNewSettings();
  AlarmStatus getAlarmStatus();
  boolean isActive();
  void virtual refresh();
  void virtual update();

protected:
  uint8_t localIndex;
  float currentValue;
  float preValue;
  int8_t currentGradient;
  int8_t gradientSign;
  MedianFilterFloat *medianValue;
  float minValue;
  float maxValue;
  SensorType type;
  String name;
  String color;
  String address;
  AlarmSetting alarmSetting;
  TemperatureCalculation_t calcTemperature;
  boolean fixedSensor;
  boolean connected;

private:
  TemperatureUnit currentUnit;
  uint8_t notificationCounter;
  static TemperatureCalculation_t typeFunctions[NUM_OF_TYPES];
  boolean settingsChanged;
  AlarmStatus cbAlarmStatus;
  boolean acknowledgedAlarm;
  float cbCurrentValue;
  float getUnitValue(float value);
  float decimalPlace(float value);
  static float calcTemperatureNTC(uint16_t rawValue, SensorType type);
  static float calcTemperaturePTx(uint16_t rawValue, SensorType type);
};