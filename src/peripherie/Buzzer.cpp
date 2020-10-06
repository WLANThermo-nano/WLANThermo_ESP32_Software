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

#include <driver/ledc.h>
#include "Buzzer.h"

#define BUZZER_TEST_INTERVAL 1000u

Buzzer::Buzzer(uint8_t ioPin, uint8_t channel) : enabled(false)
{
  this->ioPin = ioPin;
  this->channel = channel;
  this->frequency = 0;
  this->testEnabled = false;
  ledcSetup(this->channel , this->frequency, 8u);
  ledcAttachPin(this->ioPin, this->channel);
}

void Buzzer::enable()
{
  if (this->enabled != true)
  {
    this->enabled = true;
    this->frequency = BUZZER_FREQUENCY;
    this->previousMillis = millis();
  }
}

void Buzzer::disable()
{
  if (this->enabled != false)
  {
    this->enabled = false;
    this->frequency = 0;
    ledcWriteTone(this->channel, this->frequency);
  }
}

void Buzzer::test()
{
  this->testEnabled = true;
  this->previousMillis = millis();
  ledcWriteTone(this->channel, BUZZER_FREQUENCY);
}

void Buzzer::update()
{
  if(this->testEnabled)
  {
    if ((millis() - this->previousMillis) >= BUZZER_TEST_INTERVAL)
    {
      this->testEnabled = false;
      ledcWriteTone(this->channel, 0);
    }
  }
  else if (this->enabled)
  {
    uint currentMillis = millis();
    if ((millis() - this->previousMillis) >= this->intervall)
    {
      if (this->frequency == 0)
      {
        this->frequency = BUZZER_FREQUENCY;
      }
      else
      {
        this->frequency = 0;
      }
      this->previousMillis = currentMillis;
      ledcWriteTone(this->channel, frequency);
    }
  }
}