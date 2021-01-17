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

// HELP: https://github.com/bblanchon/ArduinoJson

#include "system/SystemBase.h"
#include "display/DisplayBase.h"
#include "WebHandler.h"
#include "API.h"
#include "DbgPrint.h"
#include <Preferences.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// React to Serial Input
void read_serial(char *buffer)
{

  // Commando auslesen
  String str(buffer);
  int index = str.indexOf(':');

  // Falls zusätzliche Attribute vorhanden
  if (index > 0)
  {
    String command = str.substring(0, index);
    IPRINTP("Serial: ");
    DPRINTLN(command);

    // Umsortieren
    for (int i = 0; i < index + 1; i++)
    {
      *buffer++;
    }
    uint8_t *PM_buffer = reinterpret_cast<uint8_t *>(buffer);

    // ADD WIFI SETTINGS
    if (command == "setnetwork")
    {
      nanoWebHandler.setNetwork(NULL, PM_buffer);
      return;
    }
    else if (command == "addnetwork")
    {
      nanoWebHandler.addNetwork(NULL, PM_buffer);
      return;
    }

    // set item string
    else if (command == "item")
    {
      Buzzer *buzzer = gSystem->getBuzzer();
      String payload((char *)buffer);
      gSystem->item.write(ItemNvsKeys::kItem, payload);

      if(buzzer)
      {
        // buzzer test
        buzzer->test();
      }
      return;
    }

    // UPDATE auf bestimmte Version
    else if (command == "update")
    {
      String payload((char *)buffer);
      if (payload.indexOf("v") == 0)
      {
        if (!gSystem->otaUpdate.checkForUpdate(payload))
        {
          gSystem->otaUpdate.requestVersion(payload);
          gSystem->otaUpdate.askUpdateInfo();
        }
      }
      else
      {
        IPRINTPLN("Update unbekannt!");
      }
      return;
    }

    // Battery MIN
    else if (command == "setbattmin")
    {
      String payload((char *)buffer);
      if ((payload.length() == 4) && gSystem->battery)
      {
        gSystem->battery->min = payload.toInt();
        gSystem->battery->saveConfig();
      }
      return;
    }

    // Battery MAX
    else if (command == "setbattmax")
    {
      String payload((char *)buffer);
      if ((payload.length() == 4) && gSystem->battery)
      {
        gSystem->battery->max = payload.toInt();
        gSystem->battery->saveConfig();
      }
      return;
    }
  }
  else
  {

    // set item string
    if (str == "item")
    {
      Serial.println(gSystem->item.read(ItemNvsKeys::kItem));
      return;
    }

    else if (str == "data")
    {
      Serial.println(API::apiData(APIDATA));
      return;
    }

    else if (str == "settings")
    {
      Serial.println(API::apiData(APISETTINGS));
      return;
    }
    /*
    else if (str == "networklist") {
      nanoWebHandler.handleWifiResult(false);
      return;
    }
    
    else if (str == "networkscan") {
      nanoWebHandler.handleWifiScan(false);
      return;
    }
*/

    else if (str == "clearwifi")
    {
      gSystem->wlan.clearCredentials();
      gSystem->restart();
      return;
    }

    else if (str == "configreset")
    {
      gSystem->resetConfig();
      return;
    }
#if defined HW_MINI_V2 || defined HW_MINI_V3
    else if (str == "enabledisplay")
    {
      gDisplay->disable(false);
      gDisplay->saveConfig();
      gSystem->restart();
      return;
    }
    else if (str == "disabledisplay")
    {
      gDisplay->disable(true);
      gDisplay->saveConfig();
      gSystem->restart();
      return;
    }
    else if (str == "calibratedisplay")
    {
      gDisplay->disable(true);
      gDisplay->calibrate();
      return;
    }
    else if (str == "removecalibrationTFT")
    {
      Preferences prefs;
      prefs.begin("TFT");
      prefs.remove("Touch");
      Serial.println("Settings::remove: Touch");
      prefs.end();
      return;
    }
    
#endif

#if defined HW_NANO_V3
    else if (str == "resetoled")
    {
      Serial.println("Reset OLED");
      pinMode(4u, OUTPUT);
      digitalWrite(4u, LOW);
      delay(100);
      digitalWrite(4u, HIGH);
      delay(100);
      return;
    }

#endif
    /*
    else if (str == "battery") {
      notification.type = 2;
      Serial.println("Test");
      return;
    }
*/
    // RESTART SYSTEM
    else if (str == "restart")
    {
      gSystem->restart();
      return;
    }

    /*
    // LET ESP SLEEP
    else if (str == "sleep") {
      display.displayOff();
      ESP.deepSleep(0);
      delay(100); // notwendig um Prozesse zu beenden
    }
*/

    // STOP PITMASTER
    else if (str == "stop")
    {
      for (uint8_t i = 0u; i < gSystem->pitmasters.count(); i++)
      {
        Pitmaster *pm = gSystem->pitmasters[i];

        if (pm != NULL)
          pm->setType(pm_off);
      }
      gSystem->pitmasters.saveConfig();
      return;
    }

    // Get free heap size
    else if (str == "heap")
    {
      Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
      return;
    }
    /*
    else if (str == "pittest") {
      pitMaster[0].active = AUTO;
      pitMaster[0].set = 110;
      pitMaster[0].channel = 1;
      pitMaster[0].pid = 0;
      pitMaster[1].active = AUTO;
      pitMaster[1].set = 30;
      pitMaster[1].channel = 2;
      pitMaster[1].pid = 1;
      
      return;
    }
*/

    // CHECK HTTP UPDATE
    else if (str == "checkupdate")
    {
      gSystem->otaUpdate.resetUpdateInfo();
      gSystem->otaUpdate.askUpdateInfo();
      return;
    }

    // Test Wifi Flash Clear
    else if (str == "erasewifi")
    {
      //EraseWiFiFlash();
      return;
    }
  }

  IPRINTP("You entered: >");
  DPRINT(buffer);
  DPRINTPLN("<");
  DPRINTPLN("Unkwown command"); // Befehl nicht erkannt
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Put together Serial Input
int readline(int readch, char *buffer, int len)
{

  static int pos = 0;
  int rpos;

  if (readch > 0)
  {
    switch (readch)
    {
    case '\n': // Ignore new-lines
      break;
    case '\r': // Return on CR
      rpos = pos;
      pos = 0; // Reset position index ready for next time
      return rpos;
    default:
      if (pos < len - 1)
      {
        buffer[pos++] = readch;
        buffer[pos] = 0;
      }
    }
  }
  return -1; // No end of line has been found, so return -1.
}
