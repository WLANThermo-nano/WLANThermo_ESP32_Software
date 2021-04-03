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

#include <Arduino.h>
#include <ArduinoJson.h>

class API
{
public:
  API();
  static void displayObj(JsonObject &jObj);
  static void deviceObj(JsonObject &jObj);
  static void systemObj(JsonObject &jObj, bool settings = false);
  static void channelAry(JsonArray &jAry, int cc);
  static void pitTyp(JsonObject &jObj);
  static void pitAry(JsonArray &jAry, int cc);
  static void pidAry(JsonArray &jAry, int cc);
  static void iotObj(JsonObject &jObj);
  static void extObj(JsonObject &jObj);
  static void updateObj(JsonObject &jObj);
  static void alexaObj(JsonObject &jObj);
  static void urlObj(JsonObject &jObj);
  static void dataObj(JsonObject &jObj, bool cloud);
  static void settingsObj(JsonObject &jObj);
  static void cloudObj(JsonObject &jObj);
  static void notificationObj(JsonObject &jObj);
  static String apiData(int typ);
  static float limit_float(float f, int i);

private:

};
