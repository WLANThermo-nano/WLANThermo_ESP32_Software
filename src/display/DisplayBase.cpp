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
#include "DisplayBase.h"

String DisplayBase::debugString;

DisplayBase::DisplayBase()
{
  this->blocked = false;
  this->orientation = DisplayOrientation::_0;
  this->modelName = "";
  this->system = gSystem;
  this->timeout = 0u;
  this->brightness = 100u;
}

void DisplayBase::init()
{
}

void DisplayBase::update()
{
}

void DisplayBase::saveConfig()
{
  JsonDocument doc;
  JsonObject json = doc.to<JsonObject>();
  json["disabled"] = this->disabled;
  json["orientation"] = (uint16_t)this->orientation;
  json["timeout"] = this->timeout;
  json["brightness"] = this->brightness;
  Settings::write(kDisplay, json);
}

void DisplayBase::loadConfig()
{
  JsonDocument doc;
  JsonObject json = Settings::read(kDisplay, doc);

  if (!json.isNull())
  {

    if (json.containsKey("disabled"))
      this->disabled = json["disabled"].as<bool>();
    if (json.containsKey("orientation"))
      this->orientation = (DisplayOrientation)json["orientation"].as<uint16_t>();
    if (json.containsKey("timeout"))
      this->timeout = json["timeout"].as<uint16_t>();
    if (json.containsKey("brightness"))
      this->brightness = json["brightness"].as<uint8_t>();
  }
}

void DisplayBase::disable(boolean disabled)
{
  this->disabled = disabled;
}

void DisplayBase::toggleOrientation()
{
  this->orientation = (DisplayOrientation::_0 == this->orientation) ? DisplayOrientation::_180 : DisplayOrientation::_0;
}

void DisplayBase::block(boolean block)
{
  this->blocked = block;
}

void DisplayBase::calibrate()
{
}