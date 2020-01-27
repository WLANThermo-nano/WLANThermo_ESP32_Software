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

#include "TemperatureGrp.h"
#include "Settings.h"

TemperatureGrp::TemperatureGrp()
{
  this->currentUnit = Celsius;
  this->addIndex = 0u;

  for (uint8_t i = 0u; i < MAX_TEMPERATURES; i++)
    temperatures[i] = NULL;
}

void TemperatureGrp::add(TemperatureBase *temperature)
{
  temperatures[addIndex++] = temperature;
}

void TemperatureGrp::update()
{
  for (uint8_t i = 0u; i < addIndex; i++)
  {
    if (temperatures[i] != NULL)
    {
      temperatures[i]->update();
      temperatures[i]->handleCallbacks();
    }
  }
}

bool TemperatureGrp::setUnit(TemperatureUnit unit)
{
  bool hasChanged = (unit != this->currentUnit) ? true : false;

  if (hasChanged)
  {
    for (uint8_t i = 0; i < MAX_TEMPERATURES; i++)
    {
      if (temperatures[i] != NULL)
      {
        temperatures[i]->setUnit(unit);
      }
    }
    this->currentUnit = unit;
  }

  return hasChanged;
}

TemperatureUnit TemperatureGrp::getUnit()
{
  return this->currentUnit;
}

TemperatureBase *TemperatureGrp::getNextActive(uint8_t index)
{
  TemperatureBase *activeTemperature = NULL;

  for (uint8_t i = index; i < MAX_TEMPERATURES; i++)
  {
    if (temperatures[i] != NULL)
    {
      if (temperatures[i]->isActive())
      {
        activeTemperature = temperatures[i];
        break;
      }
    }
  }

  return activeTemperature;
}

uint8_t TemperatureGrp::count()
{
  return this->addIndex;
}

boolean TemperatureGrp::hasAlarm()
{
  boolean hasAlarm = false;

  for (uint8_t i = 0; i < MAX_TEMPERATURES; i++)
  {
    if (temperatures[i] != NULL)
    {
      if (temperatures[i]->getAlarmStatus() != NoAlarm)
      {
        hasAlarm = true;
        break;
      }
    }
  }
  return hasAlarm;
}

void TemperatureGrp::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kChannels, &jsonBuffer);

  if (json.success())
  {
    if (json.containsKey("temp_unit"))
      this->setUnit((TemperatureUnit)json["temp_unit"].asString()[0u]);
    else
      return;

    for (int i = 0; i < MAX_TEMPERATURES; i++)
    {
      TemperatureBase *temperature = temperatures[i];
      if (temperature != NULL)
      {
        temperature->setName(json["tname"][i].asString());
        temperature->setType(json["ttyp"][i]);
        temperature->setMinValue(json["tmin"][i]);
        temperature->setMaxValue(json["tmax"][i]);
        temperature->setAlarmSetting((AlarmSetting)json["talarm"][i].as<uint8_t>());
        temperature->setColor(json["tcolor"][i].asString());
      }
    }
  }
}

void TemperatureGrp::saveConfig()
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();

  json["temp_unit"] = String((char)currentUnit);

  JsonArray &_name = json.createNestedArray("tname");
  JsonArray &_typ = json.createNestedArray("ttyp");
  JsonArray &_min = json.createNestedArray("tmin");
  JsonArray &_max = json.createNestedArray("tmax");
  JsonArray &_alarm = json.createNestedArray("talarm");
  JsonArray &_color = json.createNestedArray("tcolor");

  for (int i = 0; i < MAX_TEMPERATURES; i++)
  {
    TemperatureBase *temperature = temperatures[i];
    if (temperature)
    {
      _name.add(temperature->getName());
      _typ.add(temperature->getType());
      _min.add(temperature->getMinValue(), 1);
      _max.add(temperature->getMaxValue(), 1);
      _alarm.add((uint8_t)temperature->getAlarmSetting());
      _color.add(temperature->getColor());
    }
  }
  Settings::write(kChannels, json);
}

TemperatureBase *TemperatureGrp::operator[](int index)
{
  return (index < MAX_TEMPERATURES) ? temperatures[index] : NULL;
}