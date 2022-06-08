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

#include "Connect.h"
#include "temperature/TemperatureBase.h"
#include "system/SystemBase.h"
#include "Settings.h"
#include "ArduinoLog.h"
#include "TaskConfig.h"
#include <byteswap.h>
#include <ESPmDNS.h>

#define READY_STATE_UNSENT 0
#define READY_STATE_OPENED 1
#define READY_STATE_HEADERS_RECEIVED 2
#define READY_STATE_LOADING 3
#define READY_STATE_DONE 4

#define HTTP_STATUS_OK 200

std::vector<ConnectDeviceType *> Connect::connectDevices;
boolean Connect::enabled = true;

AsyncHTTPRequest deviceClient = AsyncHTTPRequest();

Connect::Connect()
{
}

void Connect::init()
{
    xTaskCreatePinnedToCore(Connect::task, "Connect::task", 10000, this, TASK_PRIORITY_CONNECT_TASK, NULL, 1);
}

void Connect::loadConfig(TemperatureGrp *temperatureGrp)
{
    DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
    JsonObject &json = Settings::read(kConnect, &jsonBuffer);

    if (json.success())
    {
        if (json.containsKey("enabled"))
        {
            enabled = json["enabled"].as<boolean>();
        }

        for (uint8_t i = 0u; i < json["tname"].size(); i++)
        {
            if (json.containsKey("taddress") && json.containsKey("tcount") && json.containsKey("tselected"))
            {
                ConnectDeviceType *connectDevice = new ConnectDeviceType();
                memset(connectDevice, 0, sizeof(ConnectDeviceType));

                // reset temperatures
                for (uint8_t i = 0; i < CONNECT_TEMPERATURE_MAX_COUNT; i++)
                {
                    connectDevice->temperatures[i] = INACTIVEVALUE;
                }

                strcpy(connectDevice->name, json["tname"][i]);
                strcpy(connectDevice->address, json["taddress"][i]);
                connectDevice->count = json["tcount"][i];
                connectDevice->selected = json["tselected"][i];

                for (uint8_t s = 0u; s < connectDevice->count; s++)
                {
                    if (connectDevice->selected & (1 << s))
                    {
                        temperatureGrp->add((uint8_t)SensorType::Connect, connectDevice->address, s);
                    }
                }
                connectDevices.push_back(connectDevice);
            }
        }
    }
}

void Connect::saveConfig()
{
    DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
    JsonObject &json = jsonBuffer.createObject();

    json["enabled"] = enabled;
    JsonArray &_name = json.createNestedArray("tname");
    JsonArray &_address = json.createNestedArray("taddress");
    JsonArray &_count = json.createNestedArray("tcount");
    JsonArray &_selected = json.createNestedArray("tselected");

    for (uint8_t i = 0u; i < connectDevices.size(); i++)
    {
        ConnectDevice *connectDevice = connectDevices[i];

        // only save device with selected channels
        if (connectDevice->selected > 0u)
        {
            _name.add(connectDevice->name);
            _address.add(connectDevice->address);
            _count.add(connectDevice->count);
            _selected.add(connectDevice->selected);
        }
    }
    Settings::write(kConnect, json);
}

void Connect::getDevices()
{
    if (false == gSystem->wlan.isConnected())
        return;

    int nrOfServices = MDNS.queryService("wlanthermo", "tcp");

    if (nrOfServices == 0)
    {
        Serial.println("No services were found.");
    }
    else
    {
        Serial.print("Number of services found: ");
        Serial.println(nrOfServices);

        for (int i = 0; i < nrOfServices; i = i + 1)
        {

            Serial.println("---------------");

            Serial.print("Hostname: ");
            Serial.println(MDNS.hostname(i));

            Serial.print("IP address: ");
            Serial.println(MDNS.IP(i));

            if (MDNS.hasTxt(i, "mac_address"))
            {
                Serial.print("MAC address: ");
                Serial.println(MDNS.txt(i, "mac_address"));

                String url = "http://" + MDNS.IP(i).toString() + "/data";
                deviceClient.onReadyStateChange(Connect::onReadyStateChange, NULL);
                deviceClient.open("GET", url.c_str());
                deviceClient.send();
            }

            Serial.println("---------------");
        }
    }
}

void Connect::onReadyStateChange(void *optParm, AsyncHTTPRequest *request, int readyState)
{
    if (READY_STATE_DONE == readyState)
    {
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(request->responseText());

        JsonArray &_channels = json["channel"].asArray();
        uint8_t channelIndex = 0u;

        connectDevices[0]->status = 1u;

        for (JsonArray::iterator itChannel = _channels.begin(); itChannel != _channels.end(); ++itChannel, channelIndex++)
        {
            JsonObject &_channel = itChannel->asObject();
            connectDevices[0]->temperatures[channelIndex] = _channel["temp"];
            Serial.println(connectDevices[0]->temperatures[channelIndex]);
        }
        
        request->abort();
    }
}

uint8_t Connect::getDeviceCount()
{
    return connectDevices.size();
}

boolean Connect::getDevice(uint8_t index, ConnectDevice *device)
{
    boolean success = false;

    if ((device != NULL) && (index < connectDevices.size()))
    {
        memcpy(device, connectDevices[index], sizeof(ConnectDevice));
        success = true;
    }

    return success;
}

void Connect::setDeviceSelected(String peerAddress, uint8_t selected)
{
    const auto isKnownDevice = [peerAddress](ConnectDevice *d) {
        return (peerAddress.equalsIgnoreCase(d->address));
    };

    auto it = std::find_if(connectDevices.begin(), connectDevices.end(), isKnownDevice);

    if (it != connectDevices.end())
    {
        (*it)->selected = selected;
    }
}

boolean Connect::isDeviceConnected(String peerAddress)
{
    boolean isConnected = false;
    const auto isKnownDevice = [peerAddress](ConnectDevice *d) {
        return (peerAddress.equalsIgnoreCase(d->address));
    };

    auto it = std::find_if(connectDevices.begin(), connectDevices.end(), isKnownDevice);

    if (it != connectDevices.end())
    {
        isConnected = (boolean)(*it)->status;
    }

    return isConnected;
}

float Connect::getTemperatureValue(String peerAddress, uint8_t index)
{
    float value = INACTIVEVALUE;
    const auto isKnownDevice = [peerAddress](ConnectDevice *d) {
        return (peerAddress.equalsIgnoreCase(d->address));
    };

    auto it = std::find_if(connectDevices.begin(), connectDevices.end(), isKnownDevice);

    if ((it != connectDevices.end()) && (index < CONNECT_TEMPERATURE_MAX_COUNT))
    {
        value = (*it)->temperatures[index];
    }

    return value;
}

void Connect::task(void *parameter)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    Connect *connect = (Connect *)parameter;

    while (1)
    {
        connect->getDevices();
        vTaskDelay(TASK_CYCLE_TIME_CONNECT_TASK);
    }
}
