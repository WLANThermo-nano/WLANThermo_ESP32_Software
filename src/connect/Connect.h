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
#include <ArduinoJson.h>
#include <asyncHTTPrequest.h>
#include "temperature/TemperatureGrp.h"

#define CONNECT_ADDRESS_STRING_MAX_SIZE 18u
#define CONNECT_NAME_STRING_MAX_SIZE 18u
#define CONNECT_TEMPERATURE_MAX_COUNT 32u

typedef float (*BleGetTemperatureValue_t)(String, uint8_t);

typedef struct ConnectDevice
{
    char name[CONNECT_ADDRESS_STRING_MAX_SIZE];
    char address[CONNECT_NAME_STRING_MAX_SIZE];
    float temperatures[CONNECT_TEMPERATURE_MAX_COUNT];
    uint8_t count;
    uint8_t selected;
    uint8_t status;
} ConnectDeviceType;

class Connect
{
public:
    Connect();
    void init();
    void loadConfig(TemperatureGrp *temperatureGrp);
    void saveConfig();
    uint8_t getDeviceCount();
    boolean getDevice(uint8_t index, ConnectDevice *device);
    void setDeviceSelected(String peerAddress, uint8_t selected);
    static boolean isDeviceConnected(String peerAddress);
    static float getTemperatureValue(String peerAddress, uint8_t index);

private:
    void getDevices();
    static void task(void *parameter);
    static void onReadyStateChange(void *optParm, asyncHTTPrequest *request, int readyState);
    static boolean enabled;
    static std::vector<ConnectDeviceType *> connectDevices;
};