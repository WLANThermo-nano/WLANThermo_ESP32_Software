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

typedef float (*BleGetTemperatureValue_t)(String, uint8_t);

class Bluetooth
{
public:
    Bluetooth();
    void init();
    static float getTemperatureValue(String peerAddress, uint8_t index);

private:
    static void dfuTxFunction(struct SFwu *fwu, uint8_t *buf, uint8_t len);
    uint8_t dfuRxFunction(uint8_t *data, int maxLen);
    void getDevices();
    void printResponseStatus();
    static void task(void *parameter);
    boolean doDfu();
    static HardwareSerial *serialBle;
    static String bleDeviceJson;
};