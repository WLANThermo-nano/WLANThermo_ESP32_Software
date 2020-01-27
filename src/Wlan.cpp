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
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "Wlan.h"
#include "Settings.h"

#define APNAME "DEVICE-AP"
#define DEFAULT_HOST "DEVICE-"

#define APPASSWORD "12345678"

#define CONNECT_TIMEOUT 5u // 5s

String Wlan::hostName = DEFAULT_HOST;
String Wlan::accessPointName = APNAME;
WlanCredentials Wlan::wlanCredentials[NUM_OF_WLAN_CREDENTIALS];
WlanCredentials Wlan::currentWlanCredentials;

Wlan::Wlan()
{
  this->connectTimeout = 0u;
  wifiState = WifiState::SoftAPNoClient;

  for (uint8_t i = 0; i < NUM_OF_WLAN_CREDENTIALS; ++i)
  {
    wlanCredentials[i].ssid[0] = '\0';
    wlanCredentials[i].password[0] = '\0';
  }

  currentWlanCredentials.ssid[0] = '\0';
  currentWlanCredentials.password[0] = '\0';
}

void Wlan::init()
{
  loadConfig();

  WiFi.setHostname(this->hostName.c_str());
  WiFi.persistent(false);

  WiFi.onEvent(onWifiConnect, WiFiEvent_t::SYSTEM_EVENT_STA_GOT_IP);
  WiFi.onEvent(onWifiDisconnect, WiFiEvent_t::SYSTEM_EVENT_STA_DISCONNECTED);
  WiFi.onEvent(onsoftAPDisconnect, WiFiEvent_t::SYSTEM_EVENT_AP_STADISCONNECTED);

  IPAddress local_IP(192, 168, 66, 1), gateway(192, 168, 66, 1), subnet(255, 255, 255, 0);
  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(this->accessPointName.c_str(), APPASSWORD, 5);

  WiFi.mode(WIFI_AP_STA);
  Serial.printf("AP: %s\n", this->accessPointName.c_str());
  Serial.printf("IP: %s\n", local_IP.toString().c_str());
  update();
}

void Wlan::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kWifi, &jsonBuffer);

  if (json.success())
  {
    uint8_t i = 0u;

    if (json.containsKey("host"))
      hostName = json["host"].asString();
    if (json.containsKey("ap"))
      accessPointName = json["ap"].asString();

    JsonArray &_wifi = json["wifi"];
    for (JsonArray::iterator it = _wifi.begin(); (it != _wifi.end()) && (i < NUM_OF_WLAN_CREDENTIALS); ++it)
    {
      strcpy(wlanCredentials[i].ssid, _wifi[i]["SSID"].asString());
      strcpy(wlanCredentials[i].password, _wifi[i]["PASS"].asString());
      Serial.printf("Wlan::loadConfig: ssid = %s, password = %s\n", wlanCredentials[i].ssid, wlanCredentials[i].password);
      i++;
    }
  }
}

void Wlan::saveConfig()
{
  if (currentWlanCredentials.ssid[0] != '\0')
  {
    for (uint8_t i = NUM_OF_WLAN_CREDENTIALS - 1u; i != 0; i--)
    {
      strcpy(wlanCredentials[i].ssid, wlanCredentials[i - 1].ssid);
      strcpy(wlanCredentials[i].password, wlanCredentials[i - 1].password);
    }

    strcpy(wlanCredentials[0].ssid, currentWlanCredentials.ssid);
    strcpy(wlanCredentials[0].password, currentWlanCredentials.password);
    currentWlanCredentials.ssid[0] = '\0';
    currentWlanCredentials.password[0] = '\0';
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();

  json["host"] = hostName;
  json["ap"] = accessPointName;

  JsonArray &array = json.createNestedArray("wifi");

  for (uint8_t i = 0; i < NUM_OF_WLAN_CREDENTIALS; ++i)
  {
    if (wlanCredentials[i].ssid[0] != '\0')
    {
      JsonObject &_wifi = array.createNestedObject();
      _wifi["SSID"] = wlanCredentials[i].ssid;
      _wifi["PASS"] = wlanCredentials[i].password;
      Serial.printf("Wlan::saveCredentials: ssid = %s, password = %s\n", wlanCredentials[i].ssid, wlanCredentials[i].password);
    }
  }

  Settings::write(kWifi, json);
}

void Wlan::clearCredentials()
{
  for (uint8_t i = 0; i < NUM_OF_WLAN_CREDENTIALS; ++i)
  {
    wlanCredentials[i].ssid[0] = '\0';
    wlanCredentials[i].password[0] = '\0';
  }

  saveConfig();
}

boolean Wlan::isConnected()
{
  return WiFi.isConnected();
}

boolean Wlan::isAP()
{
  return (WiFi.getMode() == WIFI_MODE_APSTA) ? true : false;
}

uint8_t Wlan::numOfAPClients()
{
  uint8_t numOfClients = 0u;

  if (isAP())
    numOfClients = WiFi.softAPgetStationNum();

  return numOfClients;
}

void Wlan::addCredentials(const char *ssid, const char *password, bool force)
{
  Serial.printf("Wlan::addCredentials: ssid = %s, password = %s, force = %d\n", ssid, password, force);
  strcpy(currentWlanCredentials.ssid, ssid);
  strcpy(currentWlanCredentials.password, password);

  if (force)
  {
    saveConfig();
  }

  if (isConnected())
    WiFi.disconnect();

  wifiState = WifiState::AddCredentials;
  WiFi.begin(ssid, password);
  connectTimeout = CONNECT_TIMEOUT;
}

void Wlan::update()
{
  //Serial.printf("Wlan::update: wifiState = %d\n", wifiState);

  switch (wifiState)
  {
  case WifiState::SoftAPNoClient:
  case WifiState::ConnectingToSTA:
  case WifiState::Disconnected:
    connectToKnownStations();
    break;
  case WifiState::ConnectedToSTA:
    wifiState = (isConnected()) ? wifiState : WifiState::Disconnected;
    rssi = WiFi.RSSI();
    break;
  case WifiState::SoftAPClientConnected:
    wifiState = (numOfAPClients()) ? wifiState : WifiState::SoftAPNoClient;
    break;
  case WifiState::AddCredentials:
    connectToNewStation();
    break;
  case WifiState::StoppingRadio:
    stopAllRadio();
    break;
  }
}

void Wlan::connectToNewStation()
{
  if (isConnected())
  {
    wifiState = WifiState::ConnectedToSTA;
  }

  if (connectTimeout)
  {
    connectTimeout--;
    return;
  }

  if (numOfAPClients() > 0u)
    wifiState = WifiState::SoftAPClientConnected;
  else if (isAP())
    wifiState = WifiState::SoftAPNoClient;
  else
    wifiState = WifiState::Disconnected;
}

void Wlan::connectToKnownStations()
{
  static uint8_t stationIndex = 0u;

  // check for state machine change
  if (isConnected())
  {
    wifiState = WifiState::ConnectedToSTA;
  }
  else if (numOfAPClients())
  {
    wifiState = WifiState::SoftAPClientConnected;
  }

  // wait for connection timeout
  // connect to next filled SSID
  for (uint8_t i = 0u; (i < NUM_OF_WLAN_CREDENTIALS) && (0u == connectTimeout); i++)
  {
    if (wlanCredentials[stationIndex].ssid[0] != '\0')
    {
      connectTimeout = CONNECT_TIMEOUT;
      WiFi.begin(wlanCredentials[stationIndex].ssid, wlanCredentials[stationIndex].password);
      Serial.printf("Wlan::connectToStations: SSID = %s, PW = %s\n", wlanCredentials[stationIndex].ssid, wlanCredentials[stationIndex].password);
    }

    stationIndex++;

    if (stationIndex >= NUM_OF_WLAN_CREDENTIALS)
      stationIndex = 0u;
  }

  connectTimeout--;
}

void Wlan::stopAllRadio()
{
  //Serial.println("** Stopping WiFi+BT");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();
  wifiState = WifiState::Stopped;
}

void Wlan::onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.printf("STA: %s\n", WiFi.SSID().c_str());
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  WiFi.mode(WIFI_STA);

  if (WiFi.SSID() == currentWlanCredentials.ssid)
  {
    saveConfig();
  }

  if (!MDNS.begin(hostName.c_str()))
  {
    Serial.println("Error MDNS!");
  }
  else
  {
    MDNS.addService("http", "tcp", 80);
  }
}

void Wlan::onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.println("wifi: disconnect");
}

void Wlan::onsoftAPDisconnect(WiFiEvent_t event, WiFiEventInfo_t info)
{
  Serial.print("NO AP: ");
}

String Wlan::getMacAddress()
{
  uint8_t mac[6];
  char macStr[18] = {0};
  WiFi.macAddress(mac);
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}

String Wlan::getHostName()
{
  return this->hostName;
}

void Wlan::setHostName(String hostName)
{
  if (hostName.length())
    this->hostName = hostName;
}

String Wlan::getAccessPointName()
{
  return this->accessPointName;
}

void Wlan::setAccessPointName(String accessPointName)
{
  if (accessPointName.length())
    this->accessPointName = accessPointName;
}

int32_t Wlan::getRssi()
{
  return rssi;
}

void Wlan::setStopRequest()
{
  wifiState = WifiState::StoppingRadio;
}
