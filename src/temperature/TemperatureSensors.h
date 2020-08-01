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
#pragma once

#include "Arduino.h"

#define NUM_OF_TYPES 18u

enum class SensorType
{
  Maverick = 0,
  FantastNeu = 1,
  Fantast = 2,
  iGrill2 = 3,
  ET73 = 4,
  PERFEKTION = 5,
  _50K = 6,
  INKBIRD = 7,
  NTC100K6A1B = 8,
  Weber6743 = 9,
  Santos = 10,
  NTC5K3A1B = 11,
  PT100 = 12,
  PT1000 = 13,
  ThermoWorks = 14,
  TypeK = 15,
  Ble = 16,
  MaverickRadio = 17
};

typedef struct
{
  SensorType type;
  const char *name;
  boolean fixed;
} SensorTypeInfoType;

const SensorTypeInfoType sensorTypeInfo[NUM_OF_TYPES] = {
    {SensorType::Maverick, "1000K/Maverick", false},
    {SensorType::FantastNeu, "Fantast-Neu", false},
    {SensorType::Fantast, "Fantast", false},
    {SensorType::iGrill2, "100K/iGrill2", false},
    {SensorType::ET73, "ET-73", false},
    {SensorType::PERFEKTION, "Perfektion", false},
    {SensorType::_50K, "50K", false},
    {SensorType::INKBIRD, "Inkbird", false},
    {SensorType::NTC100K6A1B, "100K6A1B", false},
    {SensorType::Weber6743, "Weber_6743", false},
    {SensorType::Santos, "Santos", false},
    {SensorType::NTC5K3A1B, "5K3A1B", false},
    {SensorType::PT100, "PT100", false},
    {SensorType::PT1000, "PT1000", false},
    {SensorType::ThermoWorks, "ThermoWorks", false},
    {SensorType::TypeK, "Typ K", true},
    {SensorType::Ble, "Bluetooth", true},
    {SensorType::MaverickRadio, "Maverick", true}};
