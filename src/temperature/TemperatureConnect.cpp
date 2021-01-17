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

#include "TemperatureConnect.h"
#include "connect/Connect.h"

TemperatureConnect::TemperatureConnect()
{
}

TemperatureConnect::TemperatureConnect(String peerAddress, uint8_t index) : TemperatureBase()
{
  this->address = peerAddress;
  this->localIndex = index;
  this->type = SensorType::Connect;
  this->fixedSensor = true;
}

void TemperatureConnect::refresh()
{
  this->connected = Connect::isDeviceConnected(this->address);

  if (this->connected)
  {
    this->currentValue = Connect::getTemperatureValue(this->address, this->localIndex);
  }
  else
  {
    this->currentValue = INACTIVEVALUE;
  }
}
