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
#include "Pitmaster.h"
#include "DbgPrint.h"
#include "math.h"
#include "ArduinoLog.h"

#define PIDKIMAX 95 // ANTI WINDUP LIMIT MAX
#define PIDKIMIN 0  // ANTI WINDUP LIMIT MIN
#define PITMIN 0    // LOWER LIMIT SET
#define PITMAX 100  // UPPER LIMIT SET
#define PITMASTERSETMIN 50
#define PITMASTERSETMAX 200

#define SSR_FREQUENCY 0.5
#define SSR_BIT_RES 16
#define SERVO_FREQUENCY 50
#define SERVO_BIT_RES 16

#define PAUSE_DEFAULT 1000u
#define PAUSE_SSR 2000u
#define FAN_BOOST_VALUE 30u

#define OPL_FALL 97   // OPEN LID LIMIT FALLING
#define OPL_RISE 100  // OPEN LID LIMIT RISING
#define OPL_PAUSE 300 // OPEN LID PAUSE
#define OPL_THRESHOLD 2

#define ATOVERTEMP 30             // AUTOTUNE OVERTEMPERATURE LIMIT
#define ATTIMELIMIT 120L * 60000L // AUTOTUNE TIMELIMIT

#define MEDIAN_SIZE 10u
#define PM_DEFAULT_DCOUNT 15u
#define PM_DEFAULT_DCOUNT_MIN 1u
#define PM_DEFAULT_DCOUNT_MAX 60u

#define JP_THRESHOLD_CELSIUS 100u
#define JP_THRESHOLD_FAHRENHEIT 212u
#define JP_THRESHOLD_RANGE 25u
#define JP_THRESHOLD_RANGE_OUT 1000u

// duty cycle calculation
// frequency: 50Hz --> 20ms
// timer resolution: 16 bit --> 65535
// resolution: 20.000us / 65535 --> 0,305us
// e.g. servo min duty cyle:  550us / 0,305...us --> 1802
// e.g. servo min duty cyle: 2250us / 0,305...us --> 7373
#define PM_SERVO_DUTY_CYCLE_DIVISOR 0.30518f
#define PM_DEFAULT_SERVO_MIN_DUTY_CYCLE 550u    //in uS
#define PM_DEFAULT_SERVO_MAX_DUTY_CYCLE 2250u   //in uS
#define PM_CONFIG_SERVO_MIN_DUTY_CYCLE 400u     //in uS
#define PM_CONFIG_SERVO_MAX_DUTY_CYCLE 2600     //in uS

uint8_t Pitmaster::ioSupply = PITMASTER_NO_SUPPLY_IO;
uint8_t Pitmaster::ioSupplyRequested = 0u;
uint8_t Pitmaster::globalIndexTracker = 0u;

Pitmaster::Pitmaster(uint8_t ioPin1, uint8_t channel1, uint8_t ioPin2, uint8_t channel2)
{
    this->profile = NULL;
    this->temperature = NULL;
    this->dutyCycleTest = NULL;
    this->autoTune = NULL;
    this->targetTemperature = PITMASTERSETMIN;
    this->type = pm_off;
    this->typeLast = pm_auto;
    this->value = 0u;
    this->previousMillis = 0u;
    this->ioPin1 = ioPin1;
    this->ioPin2 = ioPin2;
    this->channel1 = channel1;
    this->channel2 = channel2;
    this->initActuator = NOAR;
    this->globalIndex = this->globalIndexTracker++;
    this->registeredCb = NULL;
    this->settingsChanged = false;
    this->registeredCbUserData = NULL;
    this->cbValue = 0u;
    this->medianValue = new MedianFilter<float>(MEDIAN_SIZE);
    this->ecount = PM_DEFAULT_DCOUNT;
    this->dCount = PM_DEFAULT_DCOUNT;
    this->edif = 0;
    this->jump = 0;

    this->servoDcMin = PM_DEFAULT_SERVO_MIN_DUTY_CYCLE;
    this->servoDcMax = PM_DEFAULT_SERVO_MAX_DUTY_CYCLE;

    memset((void *)&this->openLid, 0u, sizeof(this->openLid));

    pinMode(this->ioPin1, OUTPUT);
    digitalWrite(this->ioPin1, LOW);

    pinMode(this->ioPin2, OUTPUT);
    digitalWrite(this->ioPin2, LOW);
}

void Pitmaster::setType(PitmasterType type)
{
    if(pm_off != type)
    {
        this->typeLast  = type;
    }

    this->type = type;
    settingsChanged = true;
}

PitmasterType Pitmaster::getType()
{
    return this->type;
}

void Pitmaster::setTypeLast(PitmasterType type)
{
    this->typeLast = type;
    settingsChanged = true;
}

PitmasterType Pitmaster::getTypeLast()
{
    return this->typeLast;
}

void Pitmaster::assignProfile(PitmasterProfile *profile)
{
    if (profile != this->profile)
    {
        this->pidReset();
        this->profile = profile;
        settingsChanged = true;
    }
}

PitmasterProfile *Pitmaster::getAssignedProfile()
{
    return this->profile;
}

void Pitmaster::assignTemperature(TemperatureBase *temperature)
{
    // Skip BLE and Maverick Radio temperatures for assignment
    /*   if ((temperature->getType() != (uint8_t)SensorType::Ble) &&
        (temperature->getType() != (uint8_t)SensorType::MaverickRadio))*/
      if (temperature->getType() != (uint8_t)SensorType::MaverickRadio)
    {
        this->temperature = temperature;
        memset((void *)&this->openLid, 0u, sizeof(this->openLid));
        settingsChanged = true;
    }
}

TemperatureBase *Pitmaster::getAssignedTemperature()
{
    return this->temperature;
}

float Pitmaster::getValue()
{
    return this->value;
}

void Pitmaster::setValue(float value)
{
    this->value = value;
}

void Pitmaster::setTargetTemperature(float temperature)
{
    this->targetTemperature = temperature;
    settingsChanged = true;
}

float Pitmaster::getTargetTemperature()
{
    return this->targetTemperature;
}

void Pitmaster::setServoMinDutyCyle(uint16_t dutyCycle)
{
    // duty cycle is inside limit?
    if((dutyCycle >= PM_CONFIG_SERVO_MIN_DUTY_CYCLE) && (dutyCycle <= PM_CONFIG_SERVO_MAX_DUTY_CYCLE))
    {
        this->servoDcMin = dutyCycle;
    }
    else
    {
        Log.error("Pitmaster::setServoMinDutyCyle: dutyCycle = %d out of range!" CR, dutyCycle);
    }
}

void Pitmaster::setServoMaxDutyCyle(uint16_t dutyCycle)
{
    // duty cycle is inside limit?
    if((dutyCycle >= PM_CONFIG_SERVO_MIN_DUTY_CYCLE) && (dutyCycle <= PM_CONFIG_SERVO_MAX_DUTY_CYCLE))
    {
        this->servoDcMax = dutyCycle;
    }
    else
    {
        Log.error("Pitmaster::setServoMaxDutyCyle: dutyCycle = %d out of range!" CR, dutyCycle);
    }
}

void Pitmaster::setDCount(uint8_t dCount)
{
    // dCount is inside limit?
    if((dCount >= PM_DEFAULT_DCOUNT_MIN) && (dCount <= PM_DEFAULT_DCOUNT_MAX))
    {
        this->dCount = dCount;
        this->ecount = dCount;
    }
    else
    {
        Log.error("Pitmaster::setDCount: dCount = %d out of range!" CR, dCount);
    }
}

void Pitmaster::registerCallback(PitmasterCallback_t callback, void *userData)
{
    this->registeredCb = callback;
    this->registeredCbUserData = userData;
}

void Pitmaster::unregisterCallback()
{
    this->registeredCb = NULL;
}

void Pitmaster::handleCallbacks()
{
    if (this->registeredCb != NULL)
    {
        if (true == settingsChanged)
        {
            this->registeredCb(this, settingsChanged, this->registeredCbUserData);
            settingsChanged = false;
        }
        else if (cbValue != value)
        {
            this->registeredCb(this, settingsChanged, this->registeredCbUserData);
            cbValue = this->value;
        }
    }
}

boolean Pitmaster::checkPause()
{
    boolean pauseDone = false;
    uint currentMillis = millis();

    // Global Pitmaster Aktor from PID-Profil
    uint8_t actuator = this->profile->actuator;

    // overwrite, if DUTYCYCLE-Test
    if (this->dutyCycleTest != NULL)
    {
        actuator = this->dutyCycleTest->actuator;
    }

    switch (actuator)
    {
    case SSR:
        this->pause = 2000u;
        break;
    default:
        this->pause = 1000u;
        break;
    }

    if (currentMillis - this->previousMillis >= this->pause)
    {
        this->previousMillis = currentMillis;
        pauseDone = true;
    }

    return pauseDone;
}

boolean Pitmaster::startDutyCycleTest(uint8_t actuator, uint8_t value)
{
    if (this->dutyCycleTest != NULL)
        return false;

    this->dutyCycleTest = new DutyCycleTest();

    this->dutyCycleTest->actuator = actuator;
    this->dutyCycleTest->value = value;
    this->dutyCycleTest->timer = millis();

    return true;
}

boolean Pitmaster::startAutoTune()
{
    if (NULL == this->autoTune)
    {
        this->autoTune = new AutoTune();
    }

    memset(this->autoTune, 0u, sizeof(AutoTune));

    this->autoTune->set = this->targetTemperature * 0.9; // ist INT damit nach unten gerundet  // SET TEMPERTURE: 10% weniger als Reserve
    float currenttemp = this->temperature->getValue();

    // macht Autotune überhaupt Sinn?
    if (this->autoTune->set - currenttemp > (this->targetTemperature * 0.05))
    {                                  // mindestens 5% von Set
        this->disableActuators(false); // SWITCH OF HEATER
        this->autoTune->run = 2;       // AUTOTUNE INITALIZED
        this->autoTune->max = this->profile->jumppw;
        PMPRINTPLN("[AT]\t Start!");
    }
    else
    {
        delete (this->autoTune);
        this->autoTune = NULL;
    }

    this->profile->autotune = false; // Zuruecksetzen

    return true;
}

boolean Pitmaster::checkDutyCycleTest()
{
    boolean testDone = true;
    uint32_t ms;
    float newValue;

    if (this->dutyCycleTest != NULL)
    {
        testDone = false;
        ms = millis();

        if ((ms - this->dutyCycleTest->timer) > 10000u)
        {
            //disableActuators(); // stop test actuator
            delete (this->dutyCycleTest);
            this->dutyCycleTest = NULL;
            testDone = true;
        }
        else
        {
            newValue = this->dutyCycleTest->value;

            // Startanlauf: bei Servo beide male zuerst in die Mitte, bei Fan nur unten
            if (ms - dutyCycleTest->timer < 1000u)
            {
                if ((this->dutyCycleTest->actuator == FAN) || (this->dutyCycleTest->actuator == SERVO))
                    newValue = 50;
            }

            switch (this->dutyCycleTest->actuator)
            {
            case FAN:
                this->controlFan(newValue, 0, 100);
                break;
            case SERVO:
                this->controlServo(newValue, 0, 100);
                break;
            case SSR:
                this->controlSSR(newValue, 0, 100);
                break;
            default:
                break;
            }
        }
    }

    return testDone;
}

boolean Pitmaster::checkAutoTune()
{
    // http://www.matthias-trier.de/EUFH_MSR_Vertriebsing_Reglerauswahl_140411.pdf

    if (NULL == this->autoTune)
        return true;

    if (this->autoTune->stop > 0)
    {
        if (this->autoTune->stop == 1)
        {                         // sauber beendet
            this->type = pm_auto; // Pitmaster in AUTO fortsetzen
            PMPRINTPLN("Autotune beendet");
        }
        else
        {
            this->type = pm_off;
        }

        //setconfig(ePIT, {}); // TODO save

        delete (this->autoTune);
        this->autoTune = NULL;
        return true;
    }

    float currentTemp = this->temperature->getValue();
    unsigned long time = millis();

    // Startbedingungen herstellen
    if (this->autoTune->run == 2)
    {

        // Phase A1 - Aktor auf MAX einschalten
        this->autoTune->value = this->autoTune->max;
        this->autoTune->run = 3;
    }
    else if (this->autoTune->run == 3)
    {

        // Phase A2 - Start Steigungstest
        if (this->autoTune->temp[0] == 0)
        {
            this->autoTune->temp[0] = currentTemp;
            this->autoTune->time[0] = time;
            //Serial.print("0: "); Serial.print(this->autoTune->time[0]); Serial.print(" | "); Serial.println(this->autoTune->temp[0]);

            // Vmax bestimmen
        }
        else if (this->autoTune->temp[1] == 0 && currentTemp < this->autoTune->set)
        {
            float vmax;
            vmax = 1000.0 * (currentTemp - this->autoTune->temp[0]) / (time - this->autoTune->time[0]);
            if (vmax > this->autoTune->vmax)
            {
                this->autoTune->vmax = vmax;
                PMPRINTP("vmax: ");
                PMPRINTLN(vmax);
            }
            this->autoTune->temp[0] = currentTemp;
            this->autoTune->time[0] = time;

            // Phase A3 - Ende Steigung, Anfang Überschwinger
        }
        else if (this->autoTune->temp[1] == 0 && currentTemp > this->autoTune->set)
        {
            this->autoTune->value = 0; // Pitmaster ruhen
            this->autoTune->temp[1] = currentTemp;
            this->autoTune->temp[2] = currentTemp;
            this->autoTune->time[1] = time;
            //Serial.print("1: "); Serial.print(this->autoTune->time[1]); Serial.print(" | "); Serial.println(this->autoTune->temp[1]);

            // Während Überschwinger
        }
        else if (this->autoTune->temp[1] > 0 && currentTemp > this->autoTune->temp[2])
        {
            this->autoTune->temp[2] = currentTemp; // Temperatur nachziehen

            // Phase A4 - Ende Überschwinger (Schwankungen ausgleichen, eventuell abhängig vom Aktor machen)
        }
        else if (this->autoTune->temp[1] > 0 && currentTemp * 1.01 < this->autoTune->temp[2])
        {
            this->autoTune->time[2] = time;
            //Serial.print("2: "); Serial.print(this->autoTune->time[2]); Serial.print(" | "); Serial.println(this->autoTune->temp[2]);
            this->autoTune->value = 25; // Teillast müsste reichen um eine wirkung zu sehen
            this->autoTune->run = 4;
        }
    }
    else
    {

        if (currentTemp > this->autoTune->temp[2])
        { // Totzeit bestimmen, wenn Temperatur wieder steigt

            // Berechnung der Verzugszeit Tt = Tu
            uint32_t Tt = (millis() - this->autoTune->time[2]) / 1000; // Wiederanlaufzeit
            PMPRINTLN(Tt);

            // dT = Überschwinger nach Abschaltung
            float dT = 1.5 * (this->autoTune->temp[2] - this->autoTune->temp[1]);
            PMPRINTLN(dT);

            // Proportionalbereich Xp aus Anstiegsgeschwindigkeit und Verzugszeit
            float Xp = 0.83 * Tt * this->autoTune->vmax * (PITMAX / this->autoTune->max);
            float K = 100.0 / Xp;
            K = max(K, 100.0f / dT);
            this->autoTune->Kp = constrain((uint32_t)(K * 10.0), 0, 1000) / 10.0; // Kp > 100 unsinnig

            // Integralzeit: Zeit zum Erreichen vom P-Anteil
            // Tn1 = (dT * 4.0)/this->autoTune->vmax;
            // Tn2 = Tt * 4;                    // mehr Verzögerung durch langsames Anwachsen
            float Tn = max(((dT * 4.0f) / this->autoTune->vmax), (Tt * 4.0f));
            PMPRINTLN(Tn);
            K = this->autoTune->Kp / Tn;                                           // Ki = Kp / (Tn)
            this->autoTune->Ki = constrain((uint32_t)(K * 100.0), 0, 100) / 100.0; // Ki > 1 ist Unsinn

            // D-Anteil über Verhältnis Überschwinger und Anstiegsgeschwindigkeit (Brems-Funktion)
            K = 7.0 * dT;                         // Faktor gibt Agressivität der Bremsfunktion vor
            K = constrain((uint16_t)(K), 0, 100); // max 100% bei max. Geschwindigkeit, sonst stocken (i beachten)
            this->autoTune->Kd = (uint16_t)(K / this->autoTune->vmax);

            PMPRINTP("[AT]\tKp: ");
            PMPRINT(this->autoTune->Kp);
            PMPRINTP(" Ki: ");
            PMPRINT(this->autoTune->Ki);
            PMPRINTP(" Kd: ");
            PMPRINTLN(this->autoTune->Kd);

            this->profile->kp = this->autoTune->Kp;
            this->profile->ki = this->autoTune->Ki;
            this->profile->kd = this->autoTune->Kd;

            PMPRINTPLN("[AT]\tFinished!");
            //disableHeater();
            this->autoTune->stop = 1;
        }
    }

    // FEHLER
    if (currentTemp > (this->autoTune->set + ATOVERTEMP))
    {
        PMPRINTPLN("f:AT OVERTEMP");
        this->disableActuators(false);
        this->autoTune->stop = 2;
    }

    if ((time - this->autoTune->time[0]) > ATTIMELIMIT)
    { // 20 Minutes
        PMPRINTPLN("f:AT TIMEOUT");
        this->disableActuators(false);
        this->autoTune->stop = 3;
    }

    this->value = this->autoTune->value;

    this->controlActuators();

    return false;
}

boolean Pitmaster::checkOpenLid()
{
    if ((pm_auto == this->type) && (true == this->profile->opl) && true == this->temperature->isActive())
    {
        if (this->temperature->getGradient() == -1)
            openLid.fall_c++;
        else
            openLid.fall_c = 0;

        // erkennen ob Temperatur wieder eingependelt oder Timeout
        if (openLid.detected)
        { // Open Lid Detected

            openLid.count--;
            openLid.fall_c = 0;

            // extremes Überschwingen vermeiden
            if (openLid.temp > this->targetTemperature && this->temperature->getValue() < this->targetTemperature)
            {    
                openLid.temp = this->targetTemperature;
                Log.notice("OPL Reference: %F" CR, (float)(openLid.temp));
            }

            if (openLid.count <= 0) // Timeout
            {
                openLid.detected = false;
                Log.notice("OPL finished: Timeout" CR);
            }    

            else if (this->temperature->getValue() > (openLid.temp * (OPL_RISE / 100.0))) // Lid Closed
            {    
                openLid.detected = false;
                Log.notice("OPL finished: Lid closed" CR);
            }
        }
        else if (openLid.fall_c == 1)
        {
            openLid.ref = (this->temperature->getPreValue() == INACTIVEVALUE) ? this->temperature->getValue() : this->temperature->getPreValue();
        }
        else if (openLid.fall_c == 2)
        {
            //Log.notice("OPL 0: %F" CR, (float)(openLid.ref));
            //Log.notice("OPL 1: %F" CR, (float)(this->temperature->getPreValue()));
            //Log.notice("OPL 2: %F" CR, (float)(this->temperature->getValue()));
        }
        else if (openLid.fall_c > 10) {
            openLid.fall_c = 0;  // Zurücksetzen wenn über max. Count
        }
        // wenn in 4 Schritten nicht unter Threshold, dann ist es auch keine Deckelöffnung
        else if (openLid.fall_c >= 3 && (openLid.ref - this->temperature->getValue()) > OPL_THRESHOLD)
        { // Opened lid detected!            
            openLid.detected = true;
            openLid.temp = openLid.ref;
            openLid.count = OPL_PAUSE; // TODO: check pause
            
            Log.notice("OPL: %d" CR, (int)(openLid.fall_c));
            Log.notice("OPL detected: %F" CR, (float)(this->temperature->getValue()));
            Log.notice("OPL Reference: %F" CR, (float)(openLid.temp));
        }
        
    }
    else
    {
        openLid.detected = false;
        openLid.fall_c = 0;
    }

    return openLid.detected;
}

boolean Pitmaster::getOPLStatus()
{
    return openLid.detected;
}

float Pitmaster::getOPLTemperature()
{
    return openLid.temp;
}

void Pitmaster::update()
{
    // Control Autotune
    //TODO
    /*if (this->autoTune->stop > 0) stopautotune(id);
  else if (this->autoTune->run == 1) startautotune(id);*/

    this->checkOpenLid();

    if (false == this->checkDutyCycleTest())
        return;

    // Check Pitmaster Pause
    if (false == this->checkPause())
        return;

    if (false == this->checkAutoTune())
        return;

    // PITMASTER TYP
    switch (this->type)
    {
    case pm_off:
        this->disableActuators(true);
        break;
    case pm_auto:
        this->value = (false == this->openLid.detected) ? this->pidCalc() : 0u;
        this->controlActuators();
        break;
    case pm_manual:
        this->controlActuators();
        break;
    }
}

void Pitmaster::controlActuators()
{
    float linkedvalue;
    
    // JUMP DROSSEL
    if (this->jump && (this->value > this->profile->jumppw))
          this->value = this->profile->jumppw;

    initActuators();

    switch (this->profile->actuator)
    {
    case FAN:
        this->enableStepUp(true);
        this->controlFan(this->value, this->profile->dcmin, this->profile->dcmax);
        break;
    case SERVO:
        this->enableStepUp(false);
        this->controlServo(this->value, this->profile->spmin, this->profile->spmax);
        break;
    case SSR:
        this->enableStepUp(true);
        this->controlSSR(this->value, this->profile->dcmin, this->profile->dcmax);
        break;
    case DAMPER:
        this->enableStepUp(true);
        this->controlFan(this->value, this->profile->dcmin, this->profile->dcmax);
        switch(this->profile->link)
        {
            case 0: // degressiv link
                linkedvalue = (this->value > 0) ? 100 : 0;
                break;
            case 1: // linear link
                linkedvalue = (ceil(this->value * 0.1)) * 10.0;
                break;
            case 2: // lowpass link
                linkedvalue = constrain(((ceil(this->value * 5 * 0.03))*34.0), 0 ,100);
                break;
            default:
                break;
        }

        //Log.notice("OUTOUT_VALUE: %F" CR, (float)(this->value));
        //Log.notice("DAMPER_VALUE: %F" CR, (float)(linkedvalue));

        this->controlServo(linkedvalue, this->profile->spmin, this->profile->spmax);

        break;
    default:
        break;
    }
}

void Pitmaster::initActuators()
{
    switch (this->profile->actuator)
    {
    case FAN:
        if (initActuator != FAN)
        {
            ledcDetachPin(this->ioPin1);
            ledcDetachPin(this->ioPin2);
            dacWrite(this->ioPin1, 0u);
            initActuator = FAN;
        }
        break;
    case SERVO:
        if (initActuator != SERVO)
        {
            dacWrite(this->ioPin1, 0u);
            ledcDetachPin(this->ioPin1);
            ledcDetachPin(this->ioPin2);
            ledcSetup(this->channel2, SERVO_FREQUENCY, SERVO_BIT_RES);
            ledcAttachPin(this->ioPin2, this->channel2);
            ledcWrite(this->channel2, 0u);
            initActuator = SERVO;
        }
        break;
    case SSR:
        if (initActuator != SSR)
        {
            dacWrite(this->ioPin1, 0u);
            ledcDetachPin(this->ioPin1);
            ledcDetachPin(this->ioPin2);
            ledcSetup(this->channel1, SSR_FREQUENCY, SSR_BIT_RES);
            ledcAttachPin(this->ioPin1, this->channel1);
            ledcWrite(this->channel1, 0u);
            initActuator = SSR;
        }
        break;
    case DAMPER:
        if (initActuator != DAMPER)
        {
            ledcDetachPin(this->ioPin1);
            ledcDetachPin(this->ioPin2);
            dacWrite(this->ioPin1, 0u);
            ledcSetup(this->channel2, SERVO_FREQUENCY, SERVO_BIT_RES);
            ledcAttachPin(this->ioPin2, this->channel2);
            ledcWrite(this->channel2, 0u);
            initActuator = DAMPER;
        }
        break;
    default:
        break;
    }
}

void Pitmaster::controlFan(float newValue, float newDcMin, float newDcMax)
{
    static float prevValue = 0;
    // limits from global actor
    uint16_t dcmin = newDcMin * 10u; // 1. Nachkommastelle
    uint16_t dcmax = newDcMax * 10u; // 1. Nachkommastelle

    dcmin = map(dcmin, 0, 1000u, 0u, 0xFFu);
    dcmax = map(dcmax, 0, 1000u, 0u, 0xFFu);

    uint32_t newDc = map(newValue, 0, 100, dcmin, dcmax);

    // boost when coming from 0
    if ((0 == prevValue) && (newValue < FAN_BOOST_VALUE) && (newValue > 0))
    {
        newDc = map(FAN_BOOST_VALUE, 0, 100, dcmin, dcmax);
    }

    if (0u == newValue)
    {
        dacWrite(this->ioPin1, 0u);
    }
    else
    {
        dacWrite(this->ioPin1, newDc);
    }

    prevValue = newValue;
}

void Pitmaster::controlServo(float newValue, float newSPMin, float newSPMax)
{
    // limits from global actor
    uint16_t spmin = newSPMin * 10u; // 1. Nachkommastelle
    uint16_t spmax = newSPMax * 10u; // 1. Nachkommastelle

    uint16_t servoPulseMin = (uint16_t)(this->servoDcMin / PM_SERVO_DUTY_CYCLE_DIVISOR);
    uint16_t servoPulseMax = (uint16_t)(this->servoDcMax / PM_SERVO_DUTY_CYCLE_DIVISOR);

    //Serial.printf("Pitmaster::controlServo: servoPulseMin = %d\n", servoPulseMin);
    //Serial.printf("Pitmaster::controlServo: servoPulseMax = %d\n", servoPulseMax);

    spmin = map(spmin, 0, 1000, servoPulseMin, servoPulseMax);
    spmax = map(spmax, 0, 1000, servoPulseMin, servoPulseMax);

    uint32_t newSp = map(newValue, 0, 100, spmin, spmax);
    uint32_t prevSp = ledcRead(this->channel2);

    if (newSp != prevSp)
    {
        ledcWrite(this->channel2, newSp);
    }
}

void Pitmaster::controlSSR(float newValue, float newDcMin, float newDcMax)
{
    static float prevValue = 0;
    // limits from global actor
    uint16_t dcmin = newDcMin * 10u; // 1. Nachkommastelle
    uint16_t dcmax = newDcMax * 10u; // 1. Nachkommastelle

    dcmin = map(dcmin, 0, 1000u, 0u, 0xFFFFu);
    dcmax = map(dcmax, 0, 1000u, 0u, 0xFFFFu);

    uint32_t newDc = map(newValue, 0, 100, dcmin, dcmax);
    uint32_t prevDc = ledcRead(this->channel1);

    prevValue = newValue;

    if (0u == newValue)
    {
        ledcWrite(this->channel1, 0u);
    }
    else if (newDc != prevDc)
    {
        ledcWrite(this->channel1, newDc);
    }
}

void Pitmaster::enableStepUp(boolean enable)
{
    if (this->ioSupply != PITMASTER_NO_SUPPLY_IO)
    {
        if (enable)
            ioSupplyRequested |= (1u << this->globalIndex);
        else
            ioSupplyRequested &= ~(1u << globalIndex);

        digitalWrite(this->ioSupply, ioSupplyRequested ? 1u : 0u);
    }
}

void Pitmaster::disableActuators(boolean allowdelay)
{

    if (true == allowdelay && (initActuator == SERVO || initActuator == DAMPER))
    {
        this->controlServo(0, this->profile->spmin, this->profile->spmax);
        initActuator = NOAR;
        Serial.println("ServoOFF");
        return;
    }

    dacWrite(this->ioPin1, 0u);
    ledcDetachPin(this->ioPin1);
    ledcDetachPin(this->ioPin2);
    digitalWrite(this->ioPin1, LOW);
    digitalWrite(this->ioPin2, LOW);

    this->enableStepUp(false);
    initActuator = NOAR;

    this->pidReset();
    memset((void *)&this->openLid, 0u, sizeof(this->openLid));
}

boolean Pitmaster::isDutyCycleTestRunning()
{
    return (NULL != this->dutyCycleTest) ? true : false;
}

boolean Pitmaster::isAutoTuneRunning()
{
    return (NULL != this->autoTune) ? true : false;
}

void Pitmaster::setSupplyPin(uint8_t ioPin)
{
    ioSupply = ioPin;
}

float Pitmaster::pidCalc()
{
    // see: http://rn-wissen.de/wiki/index.php/Regelungstechnik
    // see: http://www.ni.com/white-paper/3782/en/

    float x = medianValue->AddValue(this->temperature->getValue()); // IST
    //Serial.printf("GetMedianValue: %f\n", x);
    float w = this->targetTemperature; // SOLL
    
    // PID Parameter
    float kp, ki, kd;
    kp = this->profile->kp;
    ki = this->profile->ki;
    kd = this->profile->kd;

    int32_t diff;
    float e;

    // Abweichung bestimmen
    switch (this->profile->actuator)
    {
    case SSR: // SSR
        diff = (w - x) * 100;
        e = diff / 100.0;
        break;

    default:
        diff = (w - x) * 10;
        e = diff / 10.0; // nur Temperaturunterschiede von >0.1°C beachten
    }

    // JUMP DETECTION
    float jpthres = JP_THRESHOLD_CELSIUS;
    if (Fahrenheit == this->temperature->getUnit())
    {
        jpthres = JP_THRESHOLD_FAHRENHEIT;
    }
    float jumpth = JP_THRESHOLD_RANGE_OUT;  // unendlich
    if (x < jpthres)
    {
        jumpth = JP_THRESHOLD_RANGE;
    }   
    if (e > jumpth)
    {
	    this->jump = true;
        this->ampch = 1;
    }
  
    // Erkennung Wellenform
    if (true == this->jump)
    {
        if (this->ampch == 1 && e <= 0) 
        {
            this->ampch = 2;
        }
        else if (this->ampch == 2 && e > 0) 
        {
            this->ampch = 3;
        }
    }

    // JUMP DEACTIVATION
    if (this->ampch == 3) // Deaktivierung sobald eine Welle durchlaufen
    {
        this->jump = false;
    }
    
    /*
    if (e > jumpth)
    {
        this->jump = true;
    }
    else if (e <= 0) // Memory (daher else if) bis Soll erreicht
    {
        this->jump = false;
    }*/

    // Proportional-Anteil
    float p_out = kp * e;

    // Differential-Anteil (Intervall-Berechnung)
    this->ecount++;
    if (this->ecount >= this->dCount) {
        this->edif = (e - this->elast) / (this->pause / 1000.0);
        this->edif = this->edif / (float) this->dCount;
        this->elast = e;
        this->ecount = 0u;
    }

    float d_out = kd * edif;

    // i-Anteil wechsl: https://github.com/WLANThermo/WLANThermo_v2/blob/b7bd6e1b56fe5659e8750c17c6dd1cd489872f6c/software/usr/sbin/wlt_2_pitmaster.py
    // Integral-Anteil
    float i_out;
    if (ki != 0)
    {

        // Sprünge im Reglerausgangswert bei Anpassung von Ki vermeiden
        if (ki != this->Ki_alt)
        {
            this->esum = (this->esum * this->Ki_alt) / ki;
            this->Ki_alt = ki;
        }

        // Anti-Windup I-Anteil
        // Keine Erhöhung I-Anteil wenn Regler bereits an der Grenze ist
        if (p_out < PITMAX)
        { //if ((p_out + d_out) < PITMAX) {
            this->esum += e * (this->pause / 1000.0);
        }

        // Anti-Windup I-Anteil (Limits)
        if (this->esum * ki > PIDKIMAX)
            this->esum = PIDKIMAX / ki;
        else if (this->esum * ki < PIDKIMIN)
            this->esum = PIDKIMIN / ki;

        i_out = ki * this->esum;
    }
    else
    {
        // Historie vergessen, da wir nach Ki = 0 von 0 aus anfangen
        this->esum = 0;
        i_out = 0;
        this->Ki_alt = 0;
    }

    // PID-Regler berechnen
    float y = p_out + i_out + d_out;
    y = constrain(y, PITMIN, PITMAX); // Auflösung am Ausgang ist begrenzt

    //PMPRINTLN("[PM]\tPID:" + String(y, 1) + "\tp:" + String(p_out, 1) + "\ti:" + String(i_out, 2) + "\td:" + String(d_out, 1));

    return y;
}

void Pitmaster::pidReset()
{
    this->esum = 0;
    this->elast = 0;
    this->Ki_alt = 0;
    this->jump = 0;
    this->ampch = 0;
}
