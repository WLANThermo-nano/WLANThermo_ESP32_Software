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

#include "TemperatureBleProxy.h"
#include "temperature/TemperatureBase.h"
#include <ArduinoJson.h>

#define BLE_UART_TX 12
#define BLE_UART_RX 14
#define BLE_BAUD 115200u

#define BLE_JSON_DEVICES "d"
#define BLE_JSON_NAME "n"
#define BLE_JSON_ADDRESS "a"
#define BLE_JSON_STATUS "s"
#define BLE_JSON_COUNT "c"
#define BLE_JSON_TEMPERATURES "t"

String TemperatureBleProxy::bleDeviceJson = "";
HardwareSerial *TemperatureBleProxy::serialBle = NULL;

TemperatureBleProxy::TemperatureBleProxy()
{
}

void TemperatureBleProxy::init()
{
  if (NULL == serialBle)
  {
    serialBle = new HardwareSerial(1);
    serialBle->begin(BLE_BAUD, SERIAL_8N1, BLE_UART_RX, BLE_UART_TX);
  }
}

void TemperatureBleProxy::update()
{
  if (NULL == serialBle)
    return;

  serialBle->println("getDevices");
  Serial.setTimeout(100);
  bleDeviceJson = serialBle->readString();
}

float TemperatureBleProxy::getCurrentValue(String peerAddress, uint8_t index)
{
  DynamicJsonBuffer jsonBuffer;

  JsonObject &json = jsonBuffer.parseObject(bleDeviceJson);

  if (!json.success())
  {
    //Serial.println("Invalid JSON");
    return INACTIVEVALUE;
  }

  if (json.containsKey(BLE_JSON_DEVICES) == false)
  {
    Serial.println("Invalid JSON: devices missing");
    return INACTIVEVALUE;
  }

  JsonArray &_devices = json[BLE_JSON_DEVICES].asArray();

  for (JsonArray::iterator itDevice = _devices.begin(); itDevice != _devices.end(); ++itDevice)
  {

    JsonObject &_device = itDevice->asObject();

    if (_device.containsKey(BLE_JSON_ADDRESS) == false)
    {
      Serial.println("Invalid JSON: address missing");
      continue;
    }

    if (peerAddress.equalsIgnoreCase(_device[BLE_JSON_ADDRESS]))
    {
      if (_device.containsKey(BLE_JSON_TEMPERATURES) == false)
      {
        Serial.println("Invalid JSON: temperatures missing");
        continue;
      }

      JsonArray &_temperatures = _device[BLE_JSON_TEMPERATURES].asArray();

      if (index < _temperatures.size())
      {
        return _temperatures[index].as<float>();
      }
    }
  }

  return INACTIVEVALUE;
}
