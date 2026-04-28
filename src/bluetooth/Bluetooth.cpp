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
#include "bleFirmwareBin_nrf52832.h"
#include "bleFirmwareDat_nrf52832.h"
#include "bleFirmwareBin_nrf52840.h"
#include "bleFirmwareDat_nrf52840.h"
#include "temperature/TemperatureBase.h"
#include "system/SystemBase.h"
#include "Settings.h"
#include "ArduinoLog.h"
#include "TaskConfig.h"
#include "Utils.h"
#include <byteswap.h>

#define BLE_BAUD 115200u

#define BLE_JSON_DEVICE "d"
#define BLE_JSON_NAME "n"
#define BLE_JSON_ADDRESS "a"
#define BLE_JSON_STATUS "s"
#define BLE_JSON_COUNT "c"
#define BLE_JSON_LAST_SEEN "ls"
#define BLE_JSON_RSSI "r"
#define BLE_JSON_SENSORS "t"
#define BLE_JSON_SENSORS_VALUE "v"
#define BLE_JSON_SENSORS_UNIT "u"

HardwareSerial *Bluetooth::serialBle = NULL;
std::vector<BleDeviceType *> Bluetooth::bleDevices;
boolean Bluetooth::enabled = true;

Bluetooth::Bluetooth(int8_t rxPin, int8_t txPin, uint8_t resetPin)
{
    serialBle = new HardwareSerial(1);
    serialBle->begin(BLE_BAUD, SERIAL_8N1, rxPin, txPin);
    this->resetPin = resetPin;
    this->builtIn = false;
    this->chipEnabled = false;
    this->isNrf52840 = false;
    pinMode(this->resetPin, OUTPUT);
    digitalWrite(this->resetPin, LOW);
}

Bluetooth::Bluetooth(HardwareSerial *serial, uint8_t resetPin)
{
    serialBle = serial;
    serialBle->begin(BLE_BAUD);
    this->resetPin = resetPin;
    this->builtIn = false;
    this->chipEnabled = false;
    this->isNrf52840 = false;
    pinMode(this->resetPin, OUTPUT);
    digitalWrite(this->resetPin, LOW);
}

void Bluetooth::init()
{
    if (this->doDfu())
    {
        builtIn = true;
        xTaskCreatePinnedToCore(Bluetooth::task, "Bluetooth::task", 6000, this, TASK_PRIORITY_BLUETOOTH_TASK, NULL, 1);
    }
}

void Bluetooth::loadConfig(TemperatureGrp *temperatureGrp)
{
    JsonDocument doc;
    JsonObject json = Settings::read(kBluetooth, doc);

    if (!json.isNull())
    {
        if (json.containsKey("enabled"))
        {
            enabled = json["enabled"].as<bool>();
        }

        for (uint8_t i = 0u; i < json["tname"].size(); i++)
        {
            if (json.containsKey("taddress") && json.containsKey("tcount") && json.containsKey("tselected"))
            {
                BleDeviceType *bleDevice = new BleDeviceType();
                memset(bleDevice, 0, sizeof(BleDeviceType));
                bleDevice->remoteIndex = BLE_DEVICE_REMOTE_INDEX_INIT;

                // reset sensors
                for (uint8_t i = 0; i < BLE_SENSORS_MAX_COUNT; i++)
                {
                    bleDevice->sensors[i] = INACTIVEVALUE;
                }

                SAFE_STRNCPY(bleDevice->name,    json["tname"][i].as<const char*>());
                SAFE_STRNCPY(bleDevice->address, json["taddress"][i].as<const char*>());
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
    JsonDocument doc;
    JsonObject json = doc.to<JsonObject>();

    json["enabled"] = enabled;
    JsonArray _name = json["tname"].to<JsonArray>();
    JsonArray _address = json["taddress"].to<JsonArray>();
    JsonArray _count = json["tcount"].to<JsonArray>();
    JsonArray _selected = json["tselected"].to<JsonArray>();

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

void Bluetooth::enable(boolean enable)
{
    this->enabled = enable;
}

void Bluetooth::enableChip(boolean enable)
{
    if (chipEnabled != enable)
    {
        chipEnabled = enable;

        if (enable)
        {
            // Toggle reset pin
            pinMode(resetPin, OUTPUT);
            digitalWrite(resetPin, LOW);
            delay(20);
            digitalWrite(resetPin, HIGH);

            // Set reset pin to input after reset, this is IMPORTANT!!!
            // Otherwise the nrf52 will reset when power save mode is enabled
            pinMode(resetPin, INPUT);
        }
        else
        {
            // Causes Recovery Mode after Reset for Mini V3 -->
            // Needs to be implemented at NRF52 firmware
            /*pinMode(resetPin, OUTPUT);
            digitalWrite(resetPin, LOW);*/
        }
    }
}

void Bluetooth::getDevices()
{
    uint32_t requestedDevices = 0u;
    bool needsDiscovery = false;

    for (uint8_t devIndex = 0u; devIndex < bleDevices.size(); devIndex++)
    {
        if (bleDevices[devIndex]->selected > 0u)
        {
            if (bleDevices[devIndex]->remoteIndex != BLE_DEVICE_REMOTE_INDEX_INIT)
            {
                requestedDevices |= (1u << bleDevices[devIndex]->remoteIndex);
            }
            else
            {
                needsDiscovery = true;
            }
        }
    }

    // When selected devices have unknown NRF index, send getDevices=0 so NRF
    // returns all nearby probes — address matching then assigns the correct index.
    if (needsDiscovery)
    {
        requestedDevices = 0u;
    }

    gSystem->wireLock();
    serialBle->printf("getDevices=%d\n", requestedDevices);
    String bleDeviceJson = serialBle->readStringUntil('\n');
    gSystem->wireRelease();
    Serial.println(bleDeviceJson);

    JsonDocument doc;
    deserializeJson(doc, bleDeviceJson);
    JsonObject json = doc.as<JsonObject>();

    if (json.isNull())
    {
        Serial.println("Invalid JSON");
        return;
    }

    if (json.containsKey(BLE_JSON_DEVICE) == false)
    {
        Serial.println("Invalid JSON: devices missing");
        return;
    }

    JsonArray _devices = json[BLE_JSON_DEVICE].as<JsonArray>();
    uint8_t deviceIndex = 0u;

    for (JsonObject _device : _devices)
    {
        if (_device.containsKey(BLE_JSON_ADDRESS) == false)
        {
            Serial.println("Invalid JSON: address missing");
            deviceIndex++;
            continue;
        }

        // check if device is known
        String deviceAddress = _device[BLE_JSON_ADDRESS].as<const char*>();
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
            SAFE_STRNCPY(bleDevice->address, deviceAddress.c_str());
            bleDevices.push_back(bleDevice);
        }

        if (BLE_DEVICE_REMOTE_INDEX_INIT == bleDevice->remoteIndex)
        {
            bleDevice->remoteIndex = deviceIndex;
        }

        if (_device.containsKey(BLE_JSON_NAME) == true)
        {
            SAFE_STRNCPY(bleDevice->name, _device[BLE_JSON_NAME].as<const char*>());
        }

        if (_device.containsKey(BLE_JSON_STATUS) == true)
        {
            bleDevice->status = _device[BLE_JSON_STATUS];
        }

        if (_device.containsKey(BLE_JSON_COUNT) == true)
        {
            bleDevice->count = _device[BLE_JSON_COUNT];
        }

        // reset sensors before updating
        for (uint8_t i = 0; i < BLE_SENSORS_MAX_COUNT; i++)
        {
            bleDevice->sensors[i] = INACTIVEVALUE;
            memset(bleDevice->units[i], 0u, BLE_SENSOR_UNIT_MAX_SIZE);
        }

        if (_device.containsKey(BLE_JSON_SENSORS) == true)
        {
            JsonArray _sensors = _device[BLE_JSON_SENSORS].as<JsonArray>();
            uint8_t sensorIndex = 0u;

            for (JsonObject _sensor : _sensors)
            {
                if (sensorIndex >= BLE_SENSORS_MAX_COUNT) break;

                if (_sensor.containsKey(BLE_JSON_SENSORS_VALUE) == true)
                {
                    bleDevice->sensors[sensorIndex] = _sensor[BLE_JSON_SENSORS_VALUE];
                }

                if (_sensor.containsKey(BLE_JSON_SENSORS_UNIT) == true)
                {
                    memcpy(bleDevice->units[sensorIndex], _sensor[BLE_JSON_SENSORS_UNIT].as<const char*>(), BLE_SENSOR_UNIT_MAX_SIZE - 1u);
                }

                sensorIndex++;
            }
        }

        deviceIndex++;
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

        // overwrite connection status when bluetooth has been disabled
        if (false == enabled)
        {
            isConnected = false;
            // reset sensors
            for (uint8_t i = 0; i < BLE_SENSORS_MAX_COUNT; i++)
            {
                (*it)->sensors[i] = INACTIVEVALUE;
            }
        }
    }

    return isConnected;
}

float Bluetooth::getSensorValue(String peerAddress, uint8_t index)
{
    float value = INACTIVEVALUE;
    const auto isKnownDevice = [peerAddress](BleDevice *d) {
        return (peerAddress.equalsIgnoreCase(d->address));
    };

    auto it = std::find_if(bleDevices.begin(), bleDevices.end(), isKnownDevice);

    if ((it != bleDevices.end()) && (index < BLE_SENSORS_MAX_COUNT))
    {
        value = (*it)->sensors[index];
    }

    return value;
}

String Bluetooth::getSensorUnit(String peerAddress, uint8_t index)
{
    char unit[BLE_SENSOR_UNIT_MAX_SIZE] = {0u};
    const auto isKnownDevice = [peerAddress](BleDevice *d) {
        return (peerAddress.equalsIgnoreCase(d->address));
    };

    auto it = std::find_if(bleDevices.begin(), bleDevices.end(), isKnownDevice);

    if ((it != bleDevices.end()) && (index < BLE_SENSORS_MAX_COUNT))
    {
        memcpy(unit, (*it)->units[index], BLE_SENSOR_UNIT_MAX_SIZE);
    }

    return unit;
}

void Bluetooth::task(void *parameter)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    Bluetooth *bluetooth = (Bluetooth *)parameter;

    while (1)
    {
        Serial.printf("Bluetooth::task, highWaterMark: %d\n", uxTaskGetStackHighWaterMark(NULL));

        if (gSystem->otaUpdate.isUpdateInProgress())
        {
            // exit task loop for better update performance
            break;
        }

        // check if bluetooth has been enabled or disabled
        if (bluetooth->enabled != bluetooth->chipEnabled)
        {
            bluetooth->enableChip(bluetooth->enabled);
        }

        // get devices only when bluetooth is enabled
        if (bluetooth->chipEnabled)
        {
            bluetooth->getDevices();
        }
        vTaskDelay(TASK_CYCLE_TIME_BLUETOOTH_TASK);
    }

    Serial.println("Delete Bluetooth task");
    vTaskDelete(NULL);
}

boolean Bluetooth::waitForBootloader(uint32_t timeoutInMs)
{
    const char bootloaderString[] = {'@', '@', 'B', 'O', 'O', 'T', 'L', 'O', 'A', 'D', 'E', 'R'};
    const uint8_t variantIndex = 8u;
    uint32_t currentMillis = millis();
    uint32_t serialStringIndex = 0u;
    uint32_t bootloaderStringIndex = 0u;
    boolean success = false;
    boolean nrf52840Check = false;

    while ((millis() - currentMillis) < timeoutInMs)
    {
        if (serialBle->available())
        {
            char currentChar = (char)serialBle->read();

            // check for nrf52840 variant
            if ((variantIndex == bootloaderStringIndex) && ('4' == currentChar))
            {
                nrf52840Check = true;
                bootloaderStringIndex++;
                continue;
            }
            else if (bootloaderString[bootloaderStringIndex++] == currentChar)
            {
                if (sizeof(bootloaderString) == bootloaderStringIndex)
                {
                    success = true;
                    this->isNrf52840 = nrf52840Check;
                    break;
                }
            }
            else
            {
                bootloaderStringIndex = 0u;
                nrf52840Check = false;
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
    boolean flashed = false;

    // Empty the RX buffer
    while (serialBle->available())
        serialBle->read();

    enableChip(true);

    // Give the bootloader some time to start
    delay(300);

    // check for startup string of bootloader
    if (waitForBootloader(500u))
    {
        Serial.println("Hello from BLE bootloader");
        Serial.println("Start flashing of BLE application");
        uint32_t flashStart = millis();

        TFwu sFwu;
        memset(&sFwu, 0, sizeof(TFwu));
        sFwu.txFunction = Bluetooth::dfuTxFunction;
        sFwu.responseTimeoutMillisec = 5000u;

        if (this->isNrf52840)
        {
            // nrf52840 firmware
            sFwu.commandObject = (uint8_t *)bleFirmwareDat_nrf52840_h;
            sFwu.commandObjectLen = sizeof(bleFirmwareDat_nrf52840_h);
            sFwu.dataObject = (uint8_t *)bleFirmwareBin_nrf52840_h;
            sFwu.dataObjectLen = sizeof(bleFirmwareBin_nrf52840_h);
            sFwu.dataObjectVer = bleFirmwareBin_nrf52840_h_version;
            Log.notice("BLE chip: nrf52840" CR);
        }
        else
        {
            // nrf52832 firmware
            sFwu.commandObject = (uint8_t *)bleFirmwareDat_nrf52832_h;
            sFwu.commandObjectLen = sizeof(bleFirmwareDat_nrf52832_h);
            sFwu.dataObject = (uint8_t *)bleFirmwareBin_nrf52832_h;
            sFwu.dataObjectLen = sizeof(bleFirmwareBin_nrf52832_h);
            sFwu.dataObjectVer = bleFirmwareBin_nrf52832_h_version;
            Log.notice("BLE chip: nrf52832" CR);
        }

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
                    Log.notice("BLE chip already up-to-date" CR);
                }
                else
                {
                    Serial.printf("\nFlashing successful (%d ms)\n", (millis() - flashStart));
                    Log.notice("BLE chip successfully flashed in %dms" CR, (millis() - flashStart));
                    flashed = true;
                }

                success = true;
                break;
            }
            else if (status == FWU_STATUS_FAILURE)
            {
                Serial.printf("\nFlashing failed = %d (%d ms)\n", sFwu.responseStatus, (millis() - flashStart));
                Log.error("BLE chip flashing failed after %dms" CR, (millis() - flashStart));
                break;
            }
        }
    }

    // Wait again for the bootloader after update
    if ((true == success) && (true == flashed))
    {
        if (waitForBootloader(5000u))
        {
            Serial.println("Hello from BLE bootloader again");
            Log.notice("BLE chip detected after flashing" CR);
        }
        else
        {
            success = false;
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