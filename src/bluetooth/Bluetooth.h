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
#include "fwu.h"
#include "temperature/TemperatureGrp.h"

#define BLUETOOTH_MAX_DEVICE_COUNT 4u

#define BLE_ADDRESS_STRING_MAX_SIZE 18u
#define BLE_NAME_STRING_MAX_SIZE 18u
#define BLE_TEMPERATURE_MAX_COUNT 8u

#define BLE_DEVICE_REMOTE_INDEX_INIT 0xFFu

typedef float (*BleGetTemperatureValue_t)(String, uint8_t);

typedef struct BleDevice
{
    char name[BLE_ADDRESS_STRING_MAX_SIZE];
    char address[BLE_NAME_STRING_MAX_SIZE];
    float temperatures[BLE_TEMPERATURE_MAX_COUNT];
    uint8_t count;
    uint8_t selected;
    uint8_t status;
    uint8_t remoteIndex;
} BleDeviceType;

class Bluetooth
{
public:
    Bluetooth(int8_t rxPin, int8_t txPin, uint8_t resetPin);
    Bluetooth(HardwareSerial *serial, uint8_t resetPin);
    void init();
    void loadConfig(TemperatureGrp *temperatureGrp);
    void saveConfig();
    boolean isBuiltIn() { return builtIn; };
    uint8_t getDeviceCount();
    boolean getDevice(uint8_t index, BleDevice *device);
    void setDeviceSelected(String peerAddress, uint8_t selected);
    static boolean isDeviceConnected(String peerAddress);
    static float getTemperatureValue(String peerAddress, uint8_t index);

private:
    static void dfuTxFunction(struct SFwu *fwu, uint8_t *buf, uint8_t len);
    uint8_t dfuRxFunction(uint8_t *data, int maxLen);
    void getDevices();
    void printResponseStatus();
    static void task(void *parameter);
    boolean doDfu();
    uint8_t resetPin;
    boolean builtIn;
    static std::vector<BleDeviceType *> bleDevices;
    static HardwareSerial *serialBle;
};