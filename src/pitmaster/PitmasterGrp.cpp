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
#include "ArduinoLog.h"

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

  Log.verbose("PitmasterGrp::update()" CR);

  for (uint8_t i = 0; i < MAX_PITMASTERS; i++)
  {
    if (pitmasters[i] != NULL)
    {
      pitmasters[i]->update();
      pitmasters[i]->handleCallbacks();
    }
  }
}
uint8_t PitmasterGrp::count()
{
  return addIndex;
}

void PitmasterGrp::loadConfig()
{
  JsonDocument doc;
  JsonObject json = Settings::read(kPitmasters, doc);

  if (!json.isNull())
  {
    JsonArray _master = json["pm"].as<JsonArray>();

    byte pitsize = 0;

    for (JsonVariant entry : _master)
    {
      Pitmaster *pm = pitmasters[pitsize];
      if (pm != NULL)
      {
        int ch = entry["ch"].as<int>();
        if (ch >= 0 && ch < (int)gSystem->temperatures.count())
          pm->assignTemperature(gSystem->temperatures[ch]);
        pm->assignProfile(gSystem->getPitmasterProfile(entry["pid"].as<uint8_t>()));
        pm->setTargetTemperature(entry["set"].as<float>());
        pm->setType((PitmasterType)entry["act"].as<uint8_t>());

        if (entry["act_last"].is<uint8_t>())
          pm->setTypeLast((PitmasterType)entry["act_last"].as<uint8_t>());

        if (pm->getType() == pm_manual)
          pm->setValue(entry["val"].as<float>());

        if (entry["dCount"].is<uint8_t>())
          pm->setDCount(entry["dCount"].as<uint8_t>());

        if (entry["servoDcMin"].is<uint16_t>())
          pm->setServoMinDutyCyle(entry["servoDcMin"].as<uint16_t>());
        if (entry["servoDcMax"].is<uint16_t>())
          pm->setServoMaxDutyCyle(entry["servoDcMax"].as<uint16_t>());
      }

      pitsize++;
    }

    JsonArray _pid = json["pid"].as<JsonArray>();

    uint8_t pidsize = 0;

    for (JsonVariant pidEntry : _pid)
    {
      PitmasterProfile *profile = gSystem->getPitmasterProfile(pidsize);
      if (profile != NULL)
      {
        profile->name = pidEntry["name"].as<const char*>();
        profile->id = pidEntry["id"];
        profile->actuator = pidEntry["aktor"];
        profile->kp = pidEntry["Kp"];
        profile->ki = pidEntry["Ki"];
        profile->kd = pidEntry["Kd"];
        profile->dcmin = pidEntry["DCmin"];
        profile->dcmax = pidEntry["DCmax"];
        profile->jumppw = pidEntry["jp"];
        profile->spmin = pidEntry["SPmin"];
        profile->spmax = pidEntry["SPmax"];
        profile->link = pidEntry["link"];
        profile->opl = pidEntry["ol"];
      }

      pidsize++;
    }
  }
}

void PitmasterGrp::saveConfig()
{
  JsonDocument doc;
  JsonObject json = doc.to<JsonObject>();
  JsonArray _master = json["pm"].to<JsonArray>();

  for (int i = 0; i < MAX_PITMASTERS; i++)
  {
    Pitmaster *pm = gSystem->pitmasters[i];
    if (pm != NULL)
    {
      JsonObject _ma = _master.add<JsonObject>();
      _ma["ch"] = TemperatureGrp::getIndex(pm->getAssignedTemperature());
      _ma["pid"] = pm->getAssignedProfile()->id;
      _ma["set"] = pm->getTargetTemperature();
      _ma["act"] = (uint8_t)pm->getType();
      _ma["act_last"] = (uint8_t)pm->getTypeLast();
      _ma["val"] = pm->getValue();
      _ma["dCount"] = pm->getDCount();
      _ma["servoDcMin"] = pm->getServoMinDutyCyle();
      _ma["servoDcMax"] = pm->getServoMaxDutyCyle();
    }
  }

  JsonArray _pit = json["pid"].to<JsonArray>();

  for (int i = 0; i < gSystem->getPitmasterProfileCount(); i++)
  {
    PitmasterProfile *profile = gSystem->getPitmasterProfile(i);
    if (profile != NULL)
    {
      JsonObject _pid = _pit.add<JsonObject>();
      _pid["name"] = profile->name;
      _pid["id"] = profile->id;
      _pid["aktor"] = profile->actuator;
      _pid["Kp"] = profile->kp;
      _pid["Ki"] = profile->ki;
      _pid["Kd"] = profile->kd;
      _pid["DCmin"] = profile->dcmin;
      _pid["DCmax"] = profile->dcmax;
      _pid["jp"] = profile->jumppw;
      _pid["SPmin"] = profile->spmin;
      _pid["SPmax"] = profile->spmax;
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

Pitmaster *PitmasterGrp::getActivePitmaster(TemperatureBase *temperature)
{
  Pitmaster *pit = NULL;

  for (uint8_t i = 0u; i < MAX_PITMASTERS; i++)
  {
    if (pitmasters[i] != NULL)
    {
      if ((pitmasters[i]->getAssignedTemperature()) == temperature && (PitmasterType::pm_auto == pitmasters[i]->getType()))
      {
        pit = pitmasters[i];
        break;
      }
    }
  }

  return pit;
}

Pitmaster *PitmasterGrp::operator[](int index)
{
  return (index < MAX_PITMASTERS) ? pitmasters[index] : NULL;
}