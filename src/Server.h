/*************************************************** 
    Copyright (C) 2016  Steffen Ochs

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

// Beispiele:
// https://github.com/spacehuhn/wifi_ducky/blob/master/esp8266_wifi_duck/esp8266_wifi_duck.ino
// WebSocketClient: https://github.com/Links2004/arduinoWebSockets/issues/119
#pragma once

#include <Arduino.h>

class AsyncWebServer;

class WServer
{
public:
    WServer();
    void init();
    static void saveConfig();
    static String getUsername();
    static String getPassword();
    static const char* getRealm(){ return "WLANThermo Device"; }
    static void setPassword(String newPassword);

private:
    void loadConfig();
    AsyncWebServer *webServer;
    static const char *username;
    static String password;
};

extern WServer gWebServer;