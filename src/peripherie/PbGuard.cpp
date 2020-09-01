/*************************************************** 
    Copyright (C) 2020  Martin Koerner

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
#include "PbGuard.h"
#include "Settings.h"

#define PBGUARD_DEFAULT_LOW_INTERVAL 5000u
#define PBGUARD_DEFAULT_HIGH_INTERVAL 1000u

PbGuard::PbGuard(uint8_t ioPin)
{
  this->ioPin = ioPin;
  this->enabled = false;
  this->state = (uint8_t)LOW;
  this->lowInterval = PBGUARD_DEFAULT_LOW_INTERVAL;
  this->highInterval = PBGUARD_DEFAULT_HIGH_INTERVAL;
  this->previousMillis = 0u;
  digitalWrite(this->ioPin, LOW);
  loadConfig();
}

void PbGuard::enable()
{
  if (this->enabled != true)
  {
    this->enabled = true;
    this->state = (uint8_t)LOW;
  }
}

void PbGuard::disable()
{
  if (this->enabled != false)
  {
    this->enabled = false;
    this->state = LOW;
    digitalWrite(this->ioPin, LOW);
  }
}

void PbGuard::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kPbGuard, &jsonBuffer);

  if (json.success())
  {

    if (json.containsKey("low"))
      this->lowInterval = json["low"];
    if (json.containsKey("high"))
      this->highInterval = json["high"];
    if (json.containsKey("enabled"))
      this->enabled = json["enabled"];
  }
}

void PbGuard::update()
{
  if (this->enabled)
  {
    uint32_t currentMillis = millis();

    if(((uint8_t)LOW) == this->state)
    {
      if ((currentMillis - this->previousMillis) >= this->lowInterval)
      {
        this->previousMillis = currentMillis;
        this->state = (uint8_t)HIGH;
        digitalWrite(this->ioPin, HIGH);
        //Serial.println("HIGH");
      }
    }
    else
    {
      if ((currentMillis - this->previousMillis) >= this->highInterval)
      {
        this->previousMillis = currentMillis;
        this->state = (uint8_t)LOW;
        digitalWrite(this->ioPin, LOW);
        //Serial.println("LOW");
      }
    }
  }
}