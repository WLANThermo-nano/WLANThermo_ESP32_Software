#pragma once
#include <stdint.h>

// Anti-windup and output limits (must match Pitmaster.cpp defines)
static const float PID_KIMAX    = 95.0f;
static const float PID_KIMIN    =  0.0f;
static const float PID_OUT_MIN  =  0.0f;
static const float PID_OUT_MAX  = 100.0f;

// Core PID output computation — no Arduino/ESP32 dependencies, directly testable.
// e        : discretised error (target - actual), sign convention: positive = too cold
// esum     : integral accumulator (in/out)
// elast    : last error used for derivative (in/out)
// Ki_alt   : last ki value; used to rescale esum on ki changes (in/out)
// edif     : last computed derivative term (in/out)
// ecount   : derivative sample counter (in/out)
// kp/ki/kd : PID gains from profile
// pause_ms : call interval in milliseconds (typically PAUSE_DEFAULT or PAUSE_SSR)
// dCount   : derivative averaging window (calls before re-computing edif)
inline float pidComputeOutput(float e,
                               float &esum, float &elast, float &Ki_alt, float &edif,
                               uint8_t &ecount,
                               float kp, float ki, float kd,
                               float pause_ms, uint8_t dCount)
{
    float p_out = kp * e;

    ecount++;
    if (ecount >= dCount) {
        edif   = (e - elast) / (pause_ms / 1000.0f);
        edif  /= (float)dCount;
        elast  = e;
        ecount = 0u;
    }
    float d_out = kd * edif;

    float i_out;
    if (ki != 0.0f) {
        if (ki != Ki_alt) {
            esum   = (esum * Ki_alt) / ki;
            Ki_alt = ki;
        }
        if (p_out < PID_OUT_MAX) {
            esum += e * (pause_ms / 1000.0f);
        }
        if      (esum * ki > PID_KIMAX) esum = PID_KIMAX / ki;
        else if (esum * ki < PID_KIMIN) esum = PID_KIMIN / ki;
        i_out = ki * esum;
    } else {
        esum   = 0.0f;
        i_out  = 0.0f;
        Ki_alt = 0.0f;
    }

    float y = p_out + i_out + d_out;
    if (y < PID_OUT_MIN) y = PID_OUT_MIN;
    if (y > PID_OUT_MAX) y = PID_OUT_MAX;
    return y;
}
