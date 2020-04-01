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
#pragma once

#include <WiFi.h>

#define NUM_OF_WLAN_CREDENTIALS 5u
#define WLAN_SSID_MAX_LENGTH 33u // 32 + '\0'
#define WLAN_PASS_MAX_LENGTH 64u // 63 + '\0'

typedef struct {
    char ssid[WLAN_SSID_MAX_LENGTH];
    char password[WLAN_PASS_MAX_LENGTH];
} WlanCredentials;

enum class WifiState
{
  SoftAPNoClient,
  SoftAPClientConnected,
  AddCredentials,
  ConnectingToSTA,
  ConnectedToSTA,
  Disconnected,
  StoppingRadio,
  Stopped
};

enum class WifiStrength
{
  High,
  Medium,
  Low,
  None
};

class Wlan
{
public:
  Wlan();
  void init();
  void update();

  String getHostName();
  void setHostName(String hostName);
  String getAccessPointName();
  void setAccessPointName(String accessPointName);
  void addCredentials(const char *ssid, const char *password, bool force = false);
  void getCredentials(WlanCredentials *credentials);
  WifiStrength getSignalStrength();
  int32_t getRssi();
  static void clearCredentials();
  String getMacAddress();
  WifiState getWifiState();
  boolean isConnected();
  boolean isAP();
  uint8_t numOfAPClients();
  static void saveConfig();
  void setStopRequest();

private:
  static void onWifiConnect(WiFiEvent_t event, WiFiEventInfo_t info);
  static void onWifiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info);
  static void onsoftAPDisconnect(WiFiEvent_t event, WiFiEventInfo_t info);
  void loadConfig();
  void connectToNewStation();
  void connectToKnownStations();
  void stopAllRadio();

  static String hostName;
  static String accessPointName;
  static WlanCredentials wlanCredentials[NUM_OF_WLAN_CREDENTIALS];
  static WlanCredentials newWlanCredentials;
  static uint8_t credentialIndex;
  int32_t rssi;
  uint16_t connectTimeout;
  WifiState wifiState;
};
