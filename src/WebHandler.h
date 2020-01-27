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
    
	  Inspired and partly taken over by Vitotai
	  https://github.com/vitotai/BrewPiLess

    HISTORY: Please refer Github History
    
 ****************************************************/
// Eventuell andere Lösung zum Auslesen des Body-Inhalts
// https://github.com/me-no-dev/ESPAsyncWebServer/issues/123
// https://github.com/me-no-dev/ESPAsyncWebServer#request-variables
#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#define TEXTPLAIN "text/plain"
#define TEXTON "aktiviert"
#define TEXTOFF "deaktiviert"
#define TEXTTRUE "true"
#define TEXTFALSE "false"
#define TEXTADD "Add"

class NanoWebHandler : public AsyncWebHandler
{
public:
  NanoWebHandler(void);
  void handleRequest(AsyncWebServerRequest *request);
  bool canHandle(AsyncWebServerRequest *request);
  void handleWifiResult(AsyncWebServerRequest *request = NULL);
  void handleWifiScan(AsyncWebServerRequest *request = NULL);
  void configreset();

private:
  void handleSettings(AsyncWebServerRequest *request);
  void handleData(AsyncWebServerRequest *request);
};

extern NanoWebHandler nanoWebHandler;

class BodyWebHandler : public AsyncWebHandler
{

public:
  BodyWebHandler(void);
  void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
  bool canHandle(AsyncWebServerRequest *request);
  bool setServerAPI(AsyncWebServerRequest *request, uint8_t *datas);
  bool setPitmaster(AsyncWebServerRequest *request, uint8_t *datas);
  bool setPID(AsyncWebServerRequest *request, uint8_t *datas);
  bool setSystem(AsyncWebServerRequest *request, uint8_t *datas);
  bool setChannels(AsyncWebServerRequest *request, uint8_t *datas);
  bool setNetwork(AsyncWebServerRequest *request, uint8_t *datas);
  bool addNetwork(AsyncWebServerRequest *request, uint8_t *datas);
  bool setIoT(AsyncWebServerRequest *request, uint8_t *datas);

private:
  int checkStringLength(String tex);
  String checkString(String tex);
  bool setPush(AsyncWebServerRequest *request, uint8_t *datas);
  bool setDCTest(AsyncWebServerRequest *request, uint8_t *datas);
};

extern BodyWebHandler bodyWebHandler;
