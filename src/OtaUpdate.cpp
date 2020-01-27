/*************************************************** 
    Copyright (C) 2016  Steffen Ochs
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

#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "OtaUpdate.h"
#include "system/SystemBase.h"
#include "display/DisplayBase.h"
#include "Settings.h"
#include "DbgPrint.h"

OtaUpdate::OtaUpdate()
{
  state = 0;

  loadConfig();

  if (state == 0)
  {
    get = "false"; // Änderungen am EE während Update
    version = "false";
  }
  autoupdate = 1;
  firmwareUrl = ""; // wird nur von der API befüllt wenn Update da ist
  spiffsUrl = "";

  state = -1; // Kontakt zur API herstellen
}

void OtaUpdate::start()
{
  xTaskCreate(OtaUpdate::task, "OtaUpdate::task", 10000, this, 100, NULL);
}

void OtaUpdate::task(void *parameter)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  OtaUpdate *otaUpdate = (OtaUpdate *)parameter;

  disableCore0WDT();
  disableCore1WDT();

  for (;;)
  {
    WiFiClient client;
    t_httpUpdate_return retVal = httpUpdate.update(client, otaUpdate->firmwareUrl);
    yield();

    switch (retVal)
    {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
    }
  }

  enableCore0WDT();
  enableCore1WDT();
  vTaskDelete(NULL);
}

void OtaUpdate::saveConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = jsonBuffer.createObject();
  json["update"] = state;
  json["getupd"] = get;
  json["autoupd"] = autoupdate;
  Settings::write(kOtaUpdate, json);
}

void OtaUpdate::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kOtaUpdate, &jsonBuffer);

  if (json.success())
  {

    if (json.containsKey("update"))
      state = json["update"];
    if (json.containsKey("autoupd"))
      autoupdate = json["autoupd"];
    if (json.containsKey("getupd"))
      get = json["getupd"].asString();
  }
}

void OtaUpdate::doHttpUpdate(const char *url)
{
  firmwareUrl = url;

  if (firmwareUrl.length() > 0)
  {
    this->start();
  }
}

void OtaUpdate::doHttpUpdate()
{

  // UPDATE beendet
  if (gSystem->otaUpdate.state == 4)
  {
    //question.typ = OTAUPDATE; TODO
    gSystem->otaUpdate.get = "false";
    gSystem->otaUpdate.state = 0;
    gSystem->otaUpdate.saveConfig();
    gSystem->otaUpdate.state = -1; // Neue Suche anstoßen
    IPRINTPLN("u:finish");         // Update finished
    return;
  }

  if (gSystem->wlan.isConnected())
  { // nur bei STA
    if (gSystem->otaUpdate.get != "false")
    {

      // UPDATE Adresse
      String adress;

      if (gSystem->otaUpdate.state == 1 || gSystem->otaUpdate.state == 3)
      { // nicht im Neuaufbau während Update
        // UPDATE 2x Wiederholen falls schief gelaufen
        if (gSystem->otaUpdate.count < 3)
          gSystem->otaUpdate.count++; // Wiederholung
        else
        {
          gSystem->otaUpdate.state = 0;
          gSystem->otaUpdate.saveConfig();
          //question.typ = OTAUPDATE; TODO
          IPRINTPLN("u:cancel"); // Update canceled
          gDisplay->block(false);
          gSystem->otaUpdate.count = 0;
          return;
        }
      }

      // UPDATE spiffs oder firmware
      gDisplay->block(true);
      t_httpUpdate_return ret;

      if (gSystem->otaUpdate.state == 1 && gSystem->otaUpdate.spiffsUrl != "")
      {                                  // erst wenn API abgefragt
        gSystem->otaUpdate.state = 2;    // Nächster Updatestatus
        gSystem->otaUpdate.saveConfig(); // SPEICHERN
        IPRINTPLN("u:SPIFFS ...");
        adress = gSystem->otaUpdate.spiffsUrl + adress; // https://.... + adress
        Serial.println(adress);
        WiFiClient client;
        ret = httpUpdate.updateSpiffs(client, adress);
      }
      else if (gSystem->otaUpdate.state == 3 && gSystem->otaUpdate.firmwareUrl != "")
      { // erst wenn API abgefragt
        gSystem->otaUpdate.state = 4;
        gSystem->otaUpdate.saveConfig(); // SPEICHERN
        IPRINTPLN("u:FW ...");
        adress = gSystem->otaUpdate.firmwareUrl + adress; // https://.... + adress
        Serial.println(adress);
        WiFiClient client;
        ret = httpUpdate.update(client, adress);
      }

      // UPDATE Ereigniskontrolle
      switch (ret)
      {
      case HTTP_UPDATE_FAILED:
        DPRINTF("[HTTP]\tUPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        DPRINTPLN("");
        if (gSystem->otaUpdate.state == 2)
          gSystem->otaUpdate.state = 1; // Spiffs wiederholen
        else
          gSystem->otaUpdate.state = 3; // Firmware wiederholen
        break;

      case HTTP_UPDATE_NO_UPDATES:
        DPRINTPLN("[HTTP]\tNO_UPDATES");
        gDisplay->block(false);
        break;

      case HTTP_UPDATE_OK:
        DPRINTPLN("[HTTP]\tUPDATE_OK");
        if (gSystem->otaUpdate.state == 2)
          ESP.restart(); // falls nach spiffs kein automatischer Restart durchgeführt wird
        break;
      }
    }
    else
    {
      if (gSystem->otaUpdate.state != 2)
      { // nicht während Neustarts im Updateprozess
        IPRINTPLN("u:no");
        gSystem->otaUpdate.state = 0; // Vorgang beenden
      }
    }
  }
}