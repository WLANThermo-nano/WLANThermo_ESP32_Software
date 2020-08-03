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
#include "TemperatureBle.h"
#include "Settings.h"
#include "bluetooth/Bluetooth.h"
#include "ArduinoLog.h"

std::vector<TemperatureBase *> TemperatureGrp::temperatures;

TemperatureGrp::TemperatureGrp()
{
  this->currentUnit = Celsius;
}

void TemperatureGrp::add(TemperatureBase *temperature)
{
  temperatures.push_back(temperature);
}

void TemperatureGrp::add(uint8_t type, String address, uint8_t localIndex)
{
  const auto isTemperature = [type, address, localIndex](TemperatureBase *t) {
    return (t->getType() == type) && (t->getAddress() == address) && (t->getLocalIndex() == localIndex);
  };

  auto it = std::find_if(temperatures.begin(), temperatures.end(), isTemperature);

  TemperatureBase *temperature = NULL;

  if (it == temperatures.end())
  {
    /* Temperature has not been found, we will add it */
    switch ((SensorType)type)
    {
    case SensorType::Ble:
      temperature = new TemperatureBle(address, localIndex);
      add(temperature);
      break;
    default:
      break;
    }
  }
}

void TemperatureGrp::remove(uint8_t type, String address, uint8_t localIndex)
{
  const auto isTemperature = [type, address, localIndex](TemperatureBase *t) {
    return (t->getType() == type) && (t->getAddress() == address) && (t->getLocalIndex() == localIndex);
  };

  auto it = std::find_if(temperatures.begin(), temperatures.end(), isTemperature);

  TemperatureBase *temperature = NULL;

  if (it != temperatures.end())
  {
    /* Temperature has been found, we will remove it */
    temperature = *it;

    switch ((SensorType)type)
    {
    case SensorType::Ble:
      delete temperature;
      temperatures.erase(it);
      break;
    default:
      break;
    }
  }
}

boolean TemperatureGrp::exists(uint8_t type, String address, uint8_t localIndex)
{
  const auto isTemperature = [type, address, localIndex](TemperatureBase *t) {
    return (t->getType() == type) && (t->getAddress() == address) && (t->getLocalIndex() == localIndex);
  };

  auto it = std::find_if(temperatures.begin(), temperatures.end(), isTemperature);

  return (it != temperatures.end());
}

uint8_t TemperatureGrp::getIndex(TemperatureBase *temperature)
{
  uint8_t globalIndex = 0u;

  const auto isTemperature = [temperature](TemperatureBase *t) {
    return (t == temperature);
  };

  auto it = std::find_if(temperatures.begin(), temperatures.end(), isTemperature);

  int index = std::distance(temperatures.begin(), it);

  if (index >= 0)
  {
    globalIndex = (uint8_t)index;
  }

  return globalIndex;
}

void TemperatureGrp::update()
{
  Log.verbose("TemperatureGrp::update()" CR);

  for (uint8_t i = 0u; i < count(); i++)
  {
    // get values form hardware
    temperatures[i]->update();
  }
}

void TemperatureGrp::refresh()
{
  Log.verbose("TemperatureGrp::refresh()" CR);

  for (uint8_t i = 0u; i < count(); i++)
  {
    temperatures[i]->refresh();

    boolean newValue = temperatures[i]->checkNewValue();
    boolean settingsChanged = temperatures[i]->checkNewSettings();

    if (newValue || settingsChanged)
    {
      for (auto const &cbData : registeredCb)
      {
        cbData.cb(i, temperatures[i], settingsChanged, cbData.userData);
      }
    }
  }
}

bool TemperatureGrp::setUnit(TemperatureUnit unit)
{
  bool hasChanged = (unit != this->currentUnit) ? true : false;

  if (hasChanged)
  {
    for (uint8_t i = 0; i < count(); i++)
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

  for (uint8_t i = index; i < count(); i++)
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

uint32_t TemperatureGrp::getActiveBits()
{
  uint32_t bits = 0u;

  TemperatureBase *activeTemperature = NULL;

  for (uint8_t i = 0u; i < count(); i++)
  {
    if (temperatures[i] != NULL)
    {
      if (temperatures[i]->isActive())
      {
        bits |= (1u << i);
      }
    }
  }

  return bits;
}

uint8_t TemperatureGrp::getActiveCount()
{
  uint8_t activeCount = 0u;

  TemperatureBase *activeTemperature = NULL;

  for (uint8_t i = 0u; i < count(); i++)
  {
    if (temperatures[i] != NULL)
    {
      if (temperatures[i]->isActive())
      {
        activeCount++;
      }
    }
  }

  return activeCount;
}

uint8_t TemperatureGrp::count()
{
  return temperatures.size();
}

boolean TemperatureGrp::hasAlarm()
{
  boolean hasAlarm = false;

  for (uint8_t i = 0; i < count(); i++)
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

void TemperatureGrp::acknowledgeAlarm()
{
  for (uint8_t i = 0; i < count(); i++)
  {
    if (temperatures[i] != NULL)
    {
      if (temperatures[i]->getAlarmStatus() != NoAlarm)
      {
        temperatures[i]->acknowledgeAlarm();
      }
    }
  }
}

void TemperatureGrp::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kChannels, &jsonBuffer);

  if (json.success())
  {
    if (json.containsKey("temp_unit"))
      this->setUnit((TemperatureUnit)json["temp_unit"].asString()[0u]);

    for (uint8_t i = 0u; i < temperatures.size(); i++)
    {
      temperatures[i]->loadConfig();
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
  JsonArray &_address = json.createNestedArray("taddress");
  JsonArray &_lindex = json.createNestedArray("tlindex");

  for (int i = 0; i < count(); i++)
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
      _address.add(temperature->getAddress());
      _lindex.add(temperature->getLocalIndex());
    }
  }
  Settings::write(kChannels, json);
}

TemperatureBase *TemperatureGrp::operator[](int index)
{
  return (index < count()) ? temperatures[index] : NULL;
}

void TemperatureGrp::registerCallback(TemperatureCallback_t callback, void *userData)
{
  TemperatureCallbackDataType newCallbackData = {callback, userData};
  registeredCb.push_back(newCallbackData);
}