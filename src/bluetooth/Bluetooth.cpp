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

#include "Bluetooth.h"
#include "bleFirmwareDat.h"
#include "bleFirmwareBin.h"
#include "temperature/TemperatureBase.h"
#include "Settings.h"
#include "TaskConfig.h"
#include <byteswap.h>

#define BLE_BAUD 115200u

#define BLE_JSON_DEVICE "d"
#define BLE_JSON_NAME "n"
#define BLE_JSON_ADDRESS "a"
#define BLE_JSON_STATUS "s"
#define BLE_JSON_COUNT "c"
#define BLE_JSON_TEMPERATURES "t"
#define BLE_JSON_LAST_SEEN "ls"
#define BLE_JSON_RSSI "r"

HardwareSerial *Bluetooth::serialBle = NULL;
std::vector<BleDeviceType *> Bluetooth::bleDevices;

Bluetooth::Bluetooth(int8_t rxPin, int8_t txPin, uint8_t resetPin)
{
    serialBle = new HardwareSerial(1);
    serialBle->begin(BLE_BAUD, SERIAL_8N1, rxPin, txPin);
    this->resetPin = resetPin;
    this->builtIn = false;
    pinMode(this->resetPin, OUTPUT);
}

Bluetooth::Bluetooth(HardwareSerial *serial, uint8_t resetPin)
{
    serialBle = serial;
    serialBle->begin(BLE_BAUD);
    this->resetPin = resetPin;
    this->builtIn = false;
    pinMode(this->resetPin, OUTPUT);
}

void Bluetooth::init()
{
    if (this->doDfu())
    {
        builtIn = true;
        xTaskCreatePinnedToCore(Bluetooth::task, "Bluetooth::task", 10000, this, TASK_PRIORITY_BLUETOOTH_TASK, NULL, 1);
    }
}

void Bluetooth::loadConfig(TemperatureGrp *temperatureGrp)
{
    DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
    JsonObject &json = Settings::read(kBluetooth, &jsonBuffer);

    if (json.success())
    {
        for (uint8_t i = 0u; i < json["tname"].size(); i++)
        {
            if (json.containsKey("taddress") && json.containsKey("tcount") && json.containsKey("tselected"))
            {
                BleDeviceType *bleDevice = new BleDeviceType();
                memset(bleDevice, 0, sizeof(BleDeviceType));
                bleDevice->remoteIndex = BLE_DEVICE_REMOTE_INDEX_INIT;

                // reset temperatures
                for (uint8_t i = 0; i < BLE_TEMPERATURE_MAX_COUNT; i++)
                {
                    bleDevice->temperatures[i] = INACTIVEVALUE;
                }

                strcpy(bleDevice->name, json["tname"][i]);
                strcpy(bleDevice->address, json["taddress"][i]);
                bleDevice->count = json["tcount"][i];
                bleDevice->selected = json["tselected"][i];

                for (uint8_t s = 0u; s < bleDevice->count; s++)
                {
                    if (bleDevice->selected & (1 << s))
                    {
                        temperatureGrp->add((uint8_t)SensorType::Ble, bleDevice->address, s);
                    }
                }
                bleDevices.push_back(bleDevice);
            }
        }
    }
}

void Bluetooth::saveConfig()
{
    DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
    JsonObject &json = jsonBuffer.createObject();

    JsonArray &_name = json.createNestedArray("tname");
    JsonArray &_address = json.createNestedArray("taddress");
    JsonArray &_count = json.createNestedArray("tcount");
    JsonArray &_selected = json.createNestedArray("tselected");

    for (uint8_t i = 0u; i < bleDevices.size(); i++)
    {
        BleDevice *bleDevice = bleDevices[i];

        // only save device with selected channels
        if (bleDevice->selected > 0u)
        {
            _name.add(bleDevice->name);
            _address.add(bleDevice->address);
            _count.add(bleDevice->count);
            _selected.add(bleDevice->selected);
        }
    }
    Settings::write(kBluetooth, json);
}

void Bluetooth::getDevices()
{
    uint32_t requestedDevices = 0u;

    for (uint8_t devIndex = 0u; devIndex < bleDevices.size(); devIndex++)
    {
        if (bleDevices[devIndex]->selected > 0u)
        {
            requestedDevices |= (1u << bleDevices[devIndex]->remoteIndex);
        }
    }

    serialBle->printf("getDevices=%d\n", requestedDevices);
    String bleDeviceJson = serialBle->readStringUntil('\n');
    Serial.println(bleDeviceJson);

    DynamicJsonBuffer jsonBuffer;

    JsonObject &json = jsonBuffer.parseObject(bleDeviceJson);

    if (!json.success())
    {
        Serial.println("Invalid JSON");
        return;
    }

    if (json.containsKey(BLE_JSON_DEVICE) == false)
    {
        Serial.println("Invalid JSON: devices missing");
        return;
    }

    JsonArray &_devices = json[BLE_JSON_DEVICE].asArray();
    uint8_t deviceIndex = 0u;

    for (JsonArray::iterator itDevice = _devices.begin(); itDevice != _devices.end(); ++itDevice, deviceIndex++)
    {

        JsonObject &_device = itDevice->asObject();

        if (_device.containsKey(BLE_JSON_ADDRESS) == false)
        {
            Serial.println("Invalid JSON: address missing");
            continue;
        }

        // check if device is known
        String deviceAddress = _device[BLE_JSON_ADDRESS];
        const auto isKnownDevice = [deviceAddress](BleDevice *d) {
            return (deviceAddress.equalsIgnoreCase(d->address));
        };

        auto it = std::find_if(bleDevices.begin(), bleDevices.end(), isKnownDevice);
        BleDevice *bleDevice;

        if (it != bleDevices.end())
        {
            bleDevice = *it;
        }
        else
        {
            bleDevice = new BleDeviceType();
            memset(bleDevice, 0, sizeof(BleDeviceType));
            bleDevice->remoteIndex = BLE_DEVICE_REMOTE_INDEX_INIT;
            strcpy(bleDevice->address, deviceAddress.c_str());
            bleDevices.push_back(bleDevice);
        }

        if (BLE_DEVICE_REMOTE_INDEX_INIT == bleDevice->remoteIndex)
        {
            bleDevice->remoteIndex = deviceIndex;
        }

        if (_device.containsKey(BLE_JSON_NAME) == true)
        {
            strcpy(bleDevice->name, _device[BLE_JSON_NAME]);
        }

        if (_device.containsKey(BLE_JSON_STATUS) == true)
        {
            bleDevice->status = _device[BLE_JSON_STATUS];
        }

        if (_device.containsKey(BLE_JSON_COUNT) == true)
        {
            bleDevice->count = _device[BLE_JSON_COUNT];
        }

        // reset temperatures before updating
        for (uint8_t i = 0; i < BLE_TEMPERATURE_MAX_COUNT; i++)
        {
            bleDevice->temperatures[i] = INACTIVEVALUE;
        }

        if (_device.containsKey(BLE_JSON_TEMPERATURES) == true)
        {
            _device[BLE_JSON_TEMPERATURES].asArray().copyTo(bleDevice->temperatures, BLE_TEMPERATURE_MAX_COUNT);
        }
    }
}

uint8_t Bluetooth::getDeviceCount()
{
    return bleDevices.size();
}

boolean Bluetooth::getDevice(uint8_t index, BleDevice *device)
{
    boolean success = false;

    if ((device != NULL) && (index < bleDevices.size()))
    {
        memcpy(device, bleDevices[index], sizeof(BleDevice));
        success = true;
    }

    return success;
}

void Bluetooth::setDeviceSelected(String peerAddress, uint8_t selected)
{
    const auto isKnownDevice = [peerAddress](BleDevice *d) {
        return (peerAddress.equalsIgnoreCase(d->address));
    };

    auto it = std::find_if(bleDevices.begin(), bleDevices.end(), isKnownDevice);

    if (it != bleDevices.end())
    {
        (*it)->selected = selected;
    }
}

boolean Bluetooth::isDeviceConnected(String peerAddress)
{
    boolean isConnected = false;
    const auto isKnownDevice = [peerAddress](BleDevice *d) {
        return (peerAddress.equalsIgnoreCase(d->address));
    };

    auto it = std::find_if(bleDevices.begin(), bleDevices.end(), isKnownDevice);

    if (it != bleDevices.end())
    {
        isConnected = (boolean)(*it)->status;
    }

    return isConnected;
}

float Bluetooth::getTemperatureValue(String peerAddress, uint8_t index)
{
    float value = INACTIVEVALUE;
    const auto isKnownDevice = [peerAddress](BleDevice *d) {
        return (peerAddress.equalsIgnoreCase(d->address));
    };

    auto it = std::find_if(bleDevices.begin(), bleDevices.end(), isKnownDevice);

    if ((it != bleDevices.end()) && (index < BLE_TEMPERATURE_MAX_COUNT))
    {
        value = (*it)->temperatures[index];
    }

    return value;
}

void Bluetooth::task(void *parameter)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    Bluetooth *bluetooth = (Bluetooth *)parameter;

    while (1)
    {
        bluetooth->getDevices();
        vTaskDelay(TASK_CYCLE_TIME_BLUETOOTH_TASK);
    }
}

boolean Bluetooth::waitForBootloader(uint32_t timeoutInMs)
{
    const char bootloaderString[] = {'@', '@', 'B', 'O', 'O', 'T', 'L', 'O', 'A', 'D', 'E', 'R'};
    uint32_t currentMillis = millis();
    uint32_t serialStringIndex = 0u;
    uint32_t bootloaderStringIndex = 0u;
    boolean success = false;

    while ((millis() - currentMillis) < timeoutInMs)
    {
        if (serialBle->available())
        {
            if (bootloaderString[bootloaderStringIndex++] == (char)serialBle->read())
            {
                if (sizeof(bootloaderString) == bootloaderStringIndex)
                {
                    success = true;
                    break;
                }
            }
            else
            {
                bootloaderStringIndex = 0u;
            }
        }
    }

    // Empty the RX buffer
    while (serialBle->available())
        serialBle->read();

    return success;
}

boolean Bluetooth::doDfu()
{
    boolean success = false;

    // Empty the RX buffer
    while (serialBle->available())
        serialBle->read();

    // Toggle reset pin
    digitalWrite(resetPin, LOW);
    delay(20);
    digitalWrite(resetPin, HIGH);

    // Give the bootloader some time to start
    delay(200);

    // check for startup string of bootloader
    if (waitForBootloader(500u))
    {
        Serial.println("Hello from BLE bootloader");

        Serial.println("Start flashing of BLE application");
        uint32_t flashStart = millis();

        TFwu sFwu;
        memset(&sFwu, 0, sizeof(TFwu));
        sFwu.commandObject = (uint8_t *)bleFirmwareDat_h;
        sFwu.commandObjectLen = sizeof(bleFirmwareDat_h);
        sFwu.dataObject = (uint8_t *)bleFirmwareBin_h;
        sFwu.dataObjectLen = sizeof(bleFirmwareBin_h);
        sFwu.dataObjectVer = bleFirmwareBin_h_version;
        sFwu.txFunction = Bluetooth::dfuTxFunction;
        sFwu.responseTimeoutMillisec = 5000u;

        fwuInit(&sFwu);
        fwuExec(&sFwu);

        while (1)
        {
            fwuCanSendData(&sFwu, 128);

            uint8_t rxBuf[4];
            uint8_t rxLen = dfuRxFunction(rxBuf, 4);
            if (rxLen > 0u)
            {
                fwuDidReceiveData(&sFwu, rxBuf, rxLen);
            }

            static uint32_t mill = millis();
            EFwuProcessStatus status = fwuYield(&sFwu, millis() - mill);
            mill = millis();

            if (FWU_STATUS_COMPLETION == status)
            {
                if (FWU_RSP_OK_NO_UPDATE == sFwu.responseStatus)
                {
                    Serial.println("\nFlashing skipped, version already up to date");
                }
                else
                {
                    Serial.printf("\nFlashing successful (%d ms)\n", (millis() - flashStart));
                }

                success = true;
                break;
            }
            else if (status == FWU_STATUS_FAILURE)
            {
                Serial.printf("\nFlashing failed = %d (%d ms)\n", sFwu.responseStatus, (millis() - flashStart));
                break;
            }
        }
    }

    serialBle->setTimeout(100);

    return success;
}

void Bluetooth::dfuTxFunction(struct SFwu *fwu, uint8_t *buf, uint8_t len)
{
    uint8_t n = 0u;
    static uint32_t bytesSent = 0u;

    /*if (len)
        Serial.print("TX: ");*/

    while (len--)
    {
        uint8_t c = *buf++;
        n = serialBle->write(&c, 1);
        //Serial.printf("%02x ", c);
        if (++bytesSent % 1000 == 0)
        {
            Serial.printf(".");
            Serial.flush();
        }
    }

    /*if (n)
        Serial.println("");*/
}

uint8_t Bluetooth::dfuRxFunction(uint8_t *data, int maxLen)
{
    uint8_t n = 0;
    uint8_t byte;
    boolean once = true;

    do
    {
        int v = serialBle->available();
        if (v)
        {
            serialBle->readBytes(&byte, 1);
            if (once)
            {
                //Serial.print("RX: ");
                once = false;
            }

            //Serial.printf("%02x ", byte);
            maxLen--;
            n++;
            *data++ = byte;
        }
        else
        {
            /*if (n)
                Serial.println("");*/

            return n;
        }

    } while (byte != 0xc0 && maxLen > 0);

    /*if (n)
        Serial.println("");*/

    return n;
}