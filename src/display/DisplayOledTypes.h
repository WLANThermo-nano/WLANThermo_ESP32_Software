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
#pragma once

typedef enum
{
  Click,
  LongClickStart,
  LongClickEnd,
  LongClickOnGoing,
  DoubleClick
} ButtonEvent;

enum class ButtonId
{
  None,
  Left,
  Right
};

enum class MenuItem
{
  Boot,

  MenuTemperature,
  MenuPitmaster,
  MenuSystem,

  TempShow,
  TempSettingsUpper,
  TempSettingsLower,
  TempSettingsType,
  TempSettingsAlarm,

  PitmasterSettingsProfile,
  PitmasterSettingsChannel,
  PitmasterSettingsTemperature,
  PitmasterSettingsType,

  SystemSettingsSSID,
  SystemSettingsIP,
  SystemSettingsHost,
  SystemSettingsUnit,
  SystemSettingsFirmwareVersion
};

enum class MenuMode
{
  Show,
  Edit,
  Set
};

enum class DisplayPopUpType
{
  IpAddress,
  Update,
  Alarm,
  None
};
