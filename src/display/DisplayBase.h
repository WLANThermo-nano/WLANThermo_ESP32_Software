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

#include "Arduino.h"
#include "system/SystemBase.h"

enum class DisplayOrientation
{
  _0 = 0,
  _180 = 180
};

class DisplayBase
{
public:
  DisplayBase();
  DisplayBase(SystemBase *system);
  virtual void init();
  virtual void hwInit(){};
  virtual void update();
  void saveConfig();
  void loadConfig();
  void disable(boolean disabled);
  void toggleOrientation();
  uint16_t getOrientation() { return (int16_t)this->orientation; };
  void block(boolean block);
  virtual String getUpdateName() { return this->modelName; };
  virtual void calibrate();
  static String debugString;

protected:
  SystemBase *system;
  boolean disabled;
  boolean blocked;
  DisplayOrientation orientation;
  String modelName;
  uint16_t timeout;
};

extern DisplayBase *gDisplay;
