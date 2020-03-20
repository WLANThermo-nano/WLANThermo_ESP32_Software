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

#include "PitmasterGrp.h"
#include "system/SystemBase.h"
#include "Settings.h"

PitmasterGrp::PitmasterGrp()
{
  this->addIndex = 0;
  this->enabled = true;

  for (uint8_t i = 0u; i < MAX_PITMASTERS; i++)
    pitmasters[i] = NULL;
}

void PitmasterGrp::add(Pitmaster *pitmaster)
{
  pitmasters[addIndex++] = pitmaster;
}

void PitmasterGrp::update()
{
  if (this->enabled != true)
    return;

  for (uint8_t i = 0; i < MAX_PITMASTERS; i++)
  {
    if (pitmasters[i] != NULL)
    {
      pitmasters[i]->update();
    }
  }
}
uint8_t PitmasterGrp::count()
{
  return addIndex;
}

void PitmasterGrp::loadConfig()
{
  DynamicJsonBuffer jsonBuffer(Settings::jsonBufferSize);
  JsonObject &json = Settings::read(kPitmasters, &jsonBuffer);

  if (json.success())
  {
    JsonArray &_master = json["pm"];

    byte pitsize = 0;

    for (JsonArray::iterator it = _master.begin(); it != _master.end(); ++it)
    {
      Pitmaster *pm = pitmasters[pitsize];
      if (pm != NULL)
      {
        pm->assignTemperature(gSystem->temperatures[_master[pitsize]["ch"]]);
        pm->assignProfile(gSystem->getPitmasterProfile(_master[pitsize]["pid"].as<uint8_t>()));
        pm->setTargetTemperature(_master[pitsize]["set"]);
        pm->setType((PitmasterType)_master[pitsize]["act"].as<uint8_t>());

        if (pm->getType() == pm_manual)
          pm->setValue(_master[pitsize]["val"].as<float>());
      }

      pitsize++;
    }

    JsonArray &_pid = json["pid"];

    uint8_t pidsize = 0;

    // Wie viele Pitmaster sind vorhanden
    for (JsonArray::iterator it = _pid.begin(); it != _pid.end(); ++it)
    {
      PitmasterProfile *profile = gSystem->getPitmasterProfile(pidsize);
      if (profile != NULL)
      {
        profile->name = _pid[pidsize]["name"].asString();
        profile->id = _pid[pidsize]["id"];
        profile->actuator = _pid[pidsize]["aktor"];
        profile->kp = _pid[pidsize]["Kp"];
        profile->ki = _pid[pidsize]["Ki"];
        profile->kd = _pid[pidsize]["Kd"];
        profile->dcmin = _pid[pidsize]["DCmin"];
        profile->dcmax = _pid[pidsize]["DCmax"];
        profile->jumppw = _pid[pidsize]["jp"];
        profile->spmin = _pid[pidsize]["SPmin"];
        profile->spmax = _pid[pidsize]["SPmax"];
        profile->link = _pid[pidsize]["link"];
        profile->opl = _pid[pidsize]["ol"];
      }

      pidsize++;
    }
  }
}

void PitmasterGrp::saveConfig()
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject &json = jsonBuffer.createObject();
  JsonArray &_master = json.createNestedArray("pm");

  for (int i = 0; i < MAX_PITMASTERS; i++)
  {
    Pitmaster *pm = gSystem->pitmasters[i];
    if (pm != NULL)
    {
      JsonObject &_ma = _master.createNestedObject();
      _ma["ch"] = pm->getAssignedTemperature()->getGlobalIndex();
      _ma["pid"] = pm->getAssignedProfile()->id;
      _ma["set"] = double_with_n_digits(pm->getTargetTemperature(), 1);
      _ma["act"] = (uint8_t)pm->getType();
      _ma["val"] = pm->getValue();
    }
  }

  JsonArray &_pit = json.createNestedArray("pid");

  for (int i = 0; i < gSystem->getPitmasterProfileCount(); i++)
  {
    PitmasterProfile *profile = gSystem->getPitmasterProfile(i);
    if (profile != NULL)
    {
      JsonObject &_pid = _pit.createNestedObject();
      _pid["name"] = profile->name;
      _pid["id"] = profile->id;
      _pid["aktor"] = profile->actuator;
      _pid["Kp"] = double_with_n_digits(profile->kp, 1);
      _pid["Ki"] = double_with_n_digits(profile->ki, 3);
      _pid["Kd"] = double_with_n_digits(profile->kd, 1);
      _pid["DCmin"] = double_with_n_digits(profile->dcmin, 1);
      _pid["DCmax"] = double_with_n_digits(profile->dcmax, 1);
      _pid["jp"] = profile->jumppw;
      _pid["SPmin"] = double_with_n_digits(profile->spmin, 1);
      _pid["SPmax"] = double_with_n_digits(profile->spmax, 1);
      _pid["link"] = profile->link;
      _pid["ol"] = profile->opl;
    }
  }

  Settings::write(kPitmasters, json);
}

void PitmasterGrp::enable(boolean enabled)
{
  if (this->enabled == enabled)
    return;

  if (enabled != true)
  {
    for (uint8_t i = 0; i < MAX_PITMASTERS; i++)
    {
      if (pitmasters[i] != NULL)
      {
        pitmasters[i]->disableActuators(false);
      }
    }
  }

  this->enabled = enabled;
}

Pitmaster *PitmasterGrp::operator[](int index)
{
  return (index < MAX_PITMASTERS) ? pitmasters[index] : NULL;
}