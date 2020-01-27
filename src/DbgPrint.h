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
#pragma once

#include <RemoteDebug.h>

#define NO_DEBUG 0
#define SERIAL_DEBUG 1
#define REMOTE_DEBUG 2

/***** CONFIGURE DEBUG SETTINGS *****/
#define DEBUG NO_DEBUG
#define MQTT_DEBUG NO_DEBUG
#define PM_DEBUG NO_DEBUG
/************************************/

#if DEBUG == REMOTE_DEBUG
extern RemoteDebug Debug;
#endif

#if DEBUG == SERIAL_DEBUG
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#define DPRINTP(...) Serial.print(F(__VA_ARGS__))
#define DPRINTPLN(...) Serial.println(F(__VA_ARGS__))
#define DPRINTF(...) Serial.printf(__VA_ARGS__)
#define IPRINT(...)         \
  Serial.print("[INFO]\t"); \
  Serial.print(__VA_ARGS__)
#define IPRINTLN(...)       \
  Serial.print("[INFO]\t"); \
  Serial.println(__VA_ARGS__)
#define IPRINTP(...)        \
  Serial.print("[INFO]\t"); \
  Serial.print(F(__VA_ARGS__))
#define IPRINTPLN(...)      \
  Serial.print("[INFO]\t"); \
  Serial.println(F(__VA_ARGS__))
#define IPRINTF(...)        \
  Serial.print("[INFO]\t"); \
  Serial.printf(__VA_ARGS__)
#elif DEBUG == REMOTE_DEBUG
#define DPRINT(...) Debug.print(__VA_ARGS__)
#define DPRINTLN(...) Debug.println(__VA_ARGS__)
#define DPRINTP(...) Debug.print(F(__VA_ARGS__))
#define DPRINTPLN(...) Debug.println(F(__VA_ARGS__))
#define DPRINTF(...) Debug.printf(__VA_ARGS__)
#define IPRINT(...)        \
  Debug.print("[INFO]\t"); \
  Debug.print(__VA_ARGS__)
#define IPRINTLN(...)      \
  Debug.print("[INFO]\t"); \
  Debug.println(__VA_ARGS__)
#define IPRINTP(...)       \
  Debug.print("[INFO]\t"); \
  Debug.print(F(__VA_ARGS__))
#define IPRINTPLN(...)     \
  Debug.print("[INFO]\t"); \
  Debug.println(F(__VA_ARGS__))
#define IPRINTF(...)       \
  Debug.print("[INFO]\t"); \
  Debug.printf(__VA_ARGS__)
#else
#define DPRINT(...)    //blank line
#define DPRINTLN(...)  //blank line
#define DPRINTP(...)   //blank line
#define DPRINTPLN(...) //blank line
#define DPRINTF(...)   //blank line
#define IPRINT(...)    //blank line
#define IPRINTLN(...)  //blank line
#define IPRINTP(...)   //blank line
#define IPRINTPLN(...) //blank line
#define IPRINTF(...)   //blank line
#endif

#if MQTT_DEBUG == SERIAL_DEBUG
#define MQPRINT(...) Serial.print(__VA_ARGS__)
#define MQPRINTLN(...) Serial.println(__VA_ARGS__)
#define MQPRINTP(...) Serial.print(F(__VA_ARGS__))
#define MQPRINTPLN(...) Serial.println(F(__VA_ARGS__))
#define MQPRINTF(...) Serial.printf(__VA_ARGS__)
#elif MQTT_DEBUG == REMOTE_DEBUG
#define MQPRINT(...) Debug.print(__VA_ARGS__)
#define MQPRINTLN(...) Debug.println(__VA_ARGS__)
#define MQPRINTP(...) Debug.print(F(__VA_ARGS__))
#define MQPRINTPLN(...) Debug.println(F(__VA_ARGS__))
#define MQPRINTF(...) Debug.printf(__VA_ARGS__)
#else
#define MQPRINT(...)    //blank line
#define MQPRINTLN(...)  //blank line
#define MQPRINTP(...)   //blank line
#define MQPRINTPLN(...) //blank line
#define MQPRINTF(...)   //blank line
#endif

#if PM_DEBUG == SERIAL_DEBUG
#define PMPRINT(...) Serial.print(__VA_ARGS__)
#define PMPRINTLN(...) Serial.println(__VA_ARGS__)
#define PMPRINTP(...) Serial.print(F(__VA_ARGS__))
#define PMPRINTPLN(...) Serial.println(F(__VA_ARGS__))
#define PMPRINTF(...) Serial.printf(__VA_ARGS__)
#elif PM_DEBUG == REMOTE_DEBUG
#define PMPRINT(...) Debug.print(__VA_ARGS__)
#define PMPRINTLN(...) Debug.println(__VA_ARGS__)
#define PMPRINTP(...) Debug.print(F(__VA_ARGS__))
#define PMPRINTPLN(...) Debug.println(F(__VA_ARGS__))
#define PMPRINTF(...) Debug.printf(__VA_ARGS__)
#else
#define PMPRINT(...)    //blank line
#define PMPRINTLN(...)  //blank line
#define PMPRINTP(...)   //blank line
#define PMPRINTPLN(...) //blank line
#define PMPRINTF(...)   //blank line
#endif

void dbgPrintInit();
void dbgPrintMain();