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
#include "bluetooth\Bluetooth.h"

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

void TemperatureGrp::addBle()
{
  for (uint8_t deviceIndex = 0u; deviceIndex < BLUETOOTH_MAX_DEVICE_COUNT; deviceIndex++)
  {
    String peerAddress = Bluetooth::getDevicePeerAddress(deviceIndex);

    if (0u == peerAddress.length())
    {
      break;
    }
    else
    {
      uint8_t temperatureCount = Bluetooth::getDeviceTemperatureCount(peerAddress);

      for (uint8_t bleTemperatureIndex = 0u; bleTemperatureIndex < temperatureCount; bleTemperatureIndex++)
      {
        boolean newTemperature = true;

        /* check if temperature is already known */
        for (uint8_t temperatureIndex = 0u; temperatureIndex < this->count(); temperatureIndex++)
        {
          if ((temperatures[temperatureIndex]->getAddress() == peerAddress) && (temperatures[temperatureIndex]->getLocalIndex() == bleTemperatureIndex))
          {
            newTemperature = false;
            break;
          }
        }

        /* add new temperature */
        if (newTemperature)
          this->addRemote((uint8_t)SensorType::Ble, peerAddress.c_str(), bleTemperatureIndex);
      }
    }
  }
}

void TemperatureGrp::removeBle()
{
  uint8_t temperatureCount = this->count();
  portMUX_TYPE mutex = portMUX_INITIALIZER_UNLOCKED;

  portENTER_CRITICAL(&mutex);

  for (uint8_t index = 0u; index < temperatureCount; index++)
  {
    if (temperatures[index]->getType() == (uint8_t)SensorType::Ble)
    {
      delete temperatures[index];
      temperatures[index] = NULL;
      addIndex--;
    }
  }

  portEXIT_CRITICAL(&mutex);
}

void TemperatureGrp::update()
{
  for (uint8_t i = 0u; i < count(); i++)
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
  return this->addIndex;
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
    else
      return;

    for (uint8_t i = 0u; i < json["tname"].size(); i++)
    {
      TemperatureBase *temperature = temperatures[i];

      // add optional remote temperatures (e.g. BLE)
      if ((temperature == NULL) && json.containsKey("taddress") && json.containsKey("tlindex"))
        temperature = addRemote(json["ttyp"][i].as<uint8_t>(), json["taddress"][i].asString(), json["tlindex"][i].as<uint8_t>());

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

TemperatureBase *TemperatureGrp::addRemote(uint8_t type, const char *address, uint8_t localIndex)
{
  TemperatureBase *temperature = NULL;

  Serial.printf("addRemote: type = %d, address = %s, localIndex = %d\n", (uint8_t)type, address, localIndex);

  switch ((SensorType)type)
  {
  case SensorType::Ble:
    temperature = new TemperatureBle(address, localIndex);
    add(temperature);
    break;
  default:
    break;
  }

  return temperature;
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