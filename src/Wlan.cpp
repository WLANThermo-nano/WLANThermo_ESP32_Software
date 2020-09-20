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
#include "Constants.h"
#include "Version.h"
#include "system/SystemBase.h"

#define APPASSWORD "12345678"

#define CONNECT_TIMEOUT 5u // 5s

String Wlan::hostName = DEFAULT_HOSTNAME;
String Wlan::accessPointName = DEFAULT_APNAME;
WlanCredentials Wlan::wlanCredentials[NUM_OF_WLAN_CREDENTIALS];
WlanCredentials Wlan::newWlanCredentials;
uint8_t Wlan::credentialIndex = 0u;

Wlan::Wlan()
{
  this->connectTimeout = 0u;
  wifiState = WifiState::SoftAPNoClient;

  memset(&wlanCredentials, 0u, sizeof(wlanCredentials));
  memset(&newWlanCredentials, 0u, sizeof(newWlanCredentials));
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
      if ((strlen(_wifi[i]["SSID"].asString()) >= WLAN_SSID_MAX_LENGTH) || (strlen(_wifi[i]["PASS"].asString()) >= WLAN_PASS_MAX_LENGTH))
      {
        Serial.println("Wlan::loadConfig: credentials invalid");
      }
      else
      {
        strcpy(wlanCredentials[i].ssid, _wifi[i]["SSID"].asString());
        strcpy(wlanCredentials[i].password, _wifi[i]["PASS"].asString());
        Serial.printf("Wlan::loadConfig: ssid = %s, password = %s\n", wlanCredentials[i].ssid, wlanCredentials[i].password);
      }
      i++;
    }
  }
}

void Wlan::saveConfig()
{
  if (newWlanCredentials.ssid[0] != '\0')
  {
    for (uint8_t i = NUM_OF_WLAN_CREDENTIALS - 1u; i != 0; i--)
    {
      strcpy(wlanCredentials[i].ssid, wlanCredentials[i - 1].ssid);
      strcpy(wlanCredentials[i].password, wlanCredentials[i - 1].password);
    }

    strcpy(wlanCredentials[0].ssid, newWlanCredentials.ssid);
    strcpy(wlanCredentials[0].password, newWlanCredentials.password);
    newWlanCredentials.ssid[0] = '\0';
    newWlanCredentials.password[0] = '\0';
    credentialIndex = 0u;
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
  if ((strlen(ssid) >= WLAN_SSID_MAX_LENGTH) || (strlen(password) >= WLAN_PASS_MAX_LENGTH))
  {
    Serial.println("Wlan::addCredentials: credentials invalid");
  }
  else
  {

    Serial.printf("Wlan::addCredentials: ssid = %s, password = %s, force = %d\n", ssid, password, force);
    strcpy(newWlanCredentials.ssid, ssid);
    strcpy(newWlanCredentials.password, password);

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
}

void Wlan::getCredentials(WlanCredentials *credentials)
{
  if (credentials != NULL)
  {
    memcpy(credentials, &wlanCredentials[credentialIndex], sizeof(WlanCredentials));
  }
}

WifiStrength Wlan::getSignalStrength()
{
  WifiStrength strength = WifiStrength::None;

  if (WiFi.isConnected())
  {
    if (WiFi.RSSI() >= -80)
    {
      strength = WifiStrength::High;
    }
    else if (WiFi.RSSI() >= -95)
    {
      strength = WifiStrength::Medium;
    }
    else if (WiFi.RSSI() >= -105)
    {
      strength = WifiStrength::Low;
    }
  }

  return strength;
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
      credentialIndex = stationIndex;
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

  if (WiFi.SSID() == newWlanCredentials.ssid)
  {
    saveConfig();
  }
  else
  {
  }

  if (!MDNS.begin(hostName.c_str()))
  {
    Serial.println("Error MDNS!");
  }
  else
  {
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("wlanthermo", "tcp", 80);
    MDNS.addServiceTxt("wlanthermo", "tcp", "device", gSystem->getDeviceName());
    MDNS.addServiceTxt("wlanthermo", "tcp", "hw_version", String("v") + String(gSystem->getHardwareVersion()));
    MDNS.addServiceTxt("wlanthermo", "tcp", "sw_version", FIRMWAREVERSION);
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

WifiState Wlan::getWifiState()
{
  return this->wifiState;
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
