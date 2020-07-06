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
#include "temperature/TemperatureBase.h"
#include "MedianFilterLib.h"

#define SERVOPULSMIN 550u
#define SERVOPULSMAX 2250u

typedef struct TPitmasterProfile
{
  String name;
  uint8_t id;
  uint8_t actuator;
  float kp;
  float ki;
  float kd;
  float dcmin; // Duty Cycle Min (FAN / SSR)
  float dcmax; // Duty Cylce Max (FAM / SSR)
  byte jumppw;
  float spmin; // Servo Pulse Min
  float spmax; // Servo Pulse Max
  byte link;   // Link between Actuators
  byte opl;
  byte autotune;
  float jumpth;
} PitmasterProfile;

typedef struct TDutyCycleTest
{
  unsigned long timer; // SHUTDOWN TIMER
  uint16_t value;      // TEST VALUE * 10
  bool dc;             // WHICH DC: min or max
  uint8_t actuator;    // WHICH ACTOR
  int8_t saved;        // PITMASTER ACTIVITY CACHE (-1 .. 100)
} DutyCycleTest;

// AUTOTUNE
struct AutoTune
{
  uint32_t set;          // BETRIEBS-TEMPERATUR
  unsigned long time[3]; // TIME VECTOR
  float temp[3];         // TEMPERATURE VECTOR
  float value;           // CURRENT AUTOTUNE VALUE
  byte run;              // WAIT FOR AUTOTUNE START: 0:off, 1:init, 2:run
  byte stop;             // STOP AUTOTUNE: 1: normal, 2: overtemp, 3: timeout
  float Kp;
  float Ki;
  float Kd;
  float vmax;
  uint8_t max; // MAXIMAL AUTOTUNE PITMASTER VALUE
};

typedef struct TOpenLid
{
  bool detected;                            // Open Lid Detected
  float ref;                                // Open Lid Temperatur Memory
  float temp;                               // Temperatur by Open Lid
  int count;                                // Open Lid Count
  int fall_c;                               // Count falling events
} OpenLid;

typedef void (*PitmasterCallback_t)(class Pitmaster *, boolean, void *);

enum PitmasterType
{
  pm_off = 0,
  pm_manual = 1,
  pm_auto = 2,
};

enum PitMasterActuator
{
  SSR,
  FAN,
  SERVO,
  DAMPER,
  NOAR
};

#define PITMASTER_NO_SUPPLY_IO 0xFFu

class Pitmaster
{
public:
  Pitmaster(uint8_t ioPin1, uint8_t channel1, uint8_t ioPin2, uint8_t channel2);
  void setType(PitmasterType type);
  PitmasterType getType();
  void assignProfile(PitmasterProfile *profile);
  PitmasterProfile *getAssignedProfile();
  void assignTemperature(TemperatureBase *temperature);
  TemperatureBase *getAssignedTemperature();
  float getValue();
  void setValue(float value);
  void setTargetTemperature(float temperature);
  float getTargetTemperature();
  uint8_t getGlobalIndex() { return this->globalIndex; };
  bool startDutyCycleTest(uint8_t actuator, uint8_t value);
  bool startAutoTune();
  float pidCalc();
  void pidReset();
  void disableActuators(boolean allowdelay);
  boolean isDutyCycleTestRunning();
  boolean isAutoTuneRunning();
  void registerCallback(PitmasterCallback_t callback, void *userData);
  void unregisterCallback();
  void handleCallbacks();
  static void setSupplyPin(uint8_t ioPin);
  void virtual update();

protected:
private:
  boolean checkPause();
  boolean checkDutyCycleTest();
  boolean checkAutoTune();
  boolean checkOpenLid();
  void controlActuators();
  void initActuators();
  void controlFan(float newValue, float newDcMin, float newDcMax);
  void controlServo(float newValue, float newSPMin, float newSPMax);
  void controlSSR(float newValue, float newDcMin, float newDcMax);
  void enableStepUp(boolean enable);
  PitmasterType type;
  PitmasterProfile *profile;
  TemperatureBase *temperature;
  DutyCycleTest *dutyCycleTest;
  AutoTune *autoTune;
  OpenLid openLid;
  float targetTemperature;
  uint8_t ioPin1;
  uint8_t ioPin2;
  uint8_t channel1;
  uint8_t channel2;
  PitMasterActuator initActuator;
  PitmasterCallback_t registeredCb;
  boolean settingsChanged;
  void *registeredCbUserData;
  float cbValue;
  MedianFilter<float> *medianValue;

  // all pitmasters objects will share one supply IO
  static uint8_t ioSupply;
  static uint8_t ioSupplyRequested;

  static uint8_t globalIndexTracker;
  uint8_t globalIndex;

  float value;

  float esum;   // PITMASTER I-PART DIFFERENZ SUM
  float elast;  // PITMASTER D-PART DIFFERENZ LAST
  float Ki_alt; // PITMASTER I-PART CACHE
  bool jump;

  uint16_t pause;
  uint previousMillis;
};
