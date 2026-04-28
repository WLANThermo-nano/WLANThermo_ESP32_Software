#include <unity.h>
#include "../../src/pitmaster/PidFormula.h"

// Helpers to set up fresh PID state
static float esum, elast, Ki_alt, edif;
static uint8_t ecount;

static void resetState() {
    esum = elast = Ki_alt = edif = 0.0f;
    ecount = 0;
}

void setUp(void) { resetState(); }
void tearDown(void) {}

// --- proportional only (ki=0, kd=0) ------------------------------------------

void test_proportional_positive_error(void) {
    float out = pidComputeOutput(10.0f,
                                 esum, elast, Ki_alt, edif, ecount,
                                 1.5f, 0.0f, 0.0f,
                                 1000.0f, 1u);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 15.0f, out);
}

void test_proportional_zero_error(void) {
    float out = pidComputeOutput(0.0f,
                                 esum, elast, Ki_alt, edif, ecount,
                                 2.0f, 0.0f, 0.0f,
                                 1000.0f, 1u);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, out);
}

// --- output clamping ---------------------------------------------------------

void test_output_clamp_max(void) {
    float out = pidComputeOutput(1000.0f,
                                 esum, elast, Ki_alt, edif, ecount,
                                 5.0f, 0.0f, 0.0f,
                                 1000.0f, 1u);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PID_OUT_MAX, out);
}

void test_output_clamp_min(void) {
    float out = pidComputeOutput(-1000.0f,
                                 esum, elast, Ki_alt, edif, ecount,
                                 5.0f, 0.0f, 0.0f,
                                 1000.0f, 1u);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, PID_OUT_MIN, out);
}

// --- integral anti-windup ----------------------------------------------------

void test_integral_accumulates(void) {
    float e = 5.0f;
    float pause_ms = 1000.0f;
    float ki = 0.5f;

    // Two calls at 1-second intervals with constant error 5°C
    pidComputeOutput(e, esum, elast, Ki_alt, edif, ecount, 0.0f, ki, 0.0f, pause_ms, 1u);
    pidComputeOutput(e, esum, elast, Ki_alt, edif, ecount, 0.0f, ki, 0.0f, pause_ms, 1u);

    // esum after 2 s of error 5: 5*1 + 5*1 = 10 → i_out = 0.5 * 10 = 5
    float out = pidComputeOutput(0.0f,
                                 esum, elast, Ki_alt, edif, ecount,
                                 0.0f, ki, 0.0f, pause_ms, 1u);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 5.0f, out);
}

void test_integral_windup_clamp(void) {
    // Drive the integral into the anti-windup ceiling (PID_KIMAX / ki)
    float ki = 1.0f;
    float pause_ms = 1000.0f;
    for (int i = 0; i < 200; i++) {
        pidComputeOutput(100.0f, esum, elast, Ki_alt, edif, ecount,
                         0.0f, ki, 0.0f, pause_ms, 1u);
    }
    // esum must be clamped to PID_KIMAX / ki = 95
    TEST_ASSERT_FLOAT_WITHIN(0.1f, PID_KIMAX / ki, esum);
    // output clamped at 100 % (i_out = ki * 95 = 95 < 100)
    float out = pidComputeOutput(0.0f, esum, elast, Ki_alt, edif, ecount,
                                 0.0f, ki, 0.0f, pause_ms, 1u);
    TEST_ASSERT_LESS_OR_EQUAL(PID_OUT_MAX, out);
}

void test_integral_reset_on_ki_zero(void) {
    // Accumulate some integral history
    esum = 50.0f;
    Ki_alt = 1.0f;

    // Next call with ki = 0 → esum must be reset
    pidComputeOutput(5.0f, esum, elast, Ki_alt, edif, ecount,
                     0.0f, 0.0f, 0.0f, 1000.0f, 1u);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, esum);
}

// --- derivative --------------------------------------------------------------

void test_derivative_on_error_change(void) {
    float kd = 1.0f;
    float pause_ms = 1000.0f;

    // First call: establishes elast = 0 (derivative computed on 1st call since ecount starts at 0)
    pidComputeOutput(0.0f, esum, elast, Ki_alt, edif, ecount, 0.0f, 0.0f, kd, pause_ms, 1u);
    resetState();  // clean slate for the controlled experiment

    // Call 1: e = 0, elast becomes 0
    pidComputeOutput(0.0f, esum, elast, Ki_alt, edif, ecount, 0.0f, 0.0f, kd, pause_ms, 1u);
    // Call 2: e = 10, derivative = (10-0)/1 = 10, d_out = 1*10 = 10
    float out = pidComputeOutput(10.0f, esum, elast, Ki_alt, edif, ecount,
                                 0.0f, 0.0f, kd, pause_ms, 1u);
    // d_out = 10, clamped to 100 at most
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 10.0f, out);
}

// --- ki change rescales esum -------------------------------------------------

void test_ki_change_rescales_esum(void) {
    // Establish esum with ki=1
    Ki_alt = 1.0f;
    esum   = 40.0f;

    // Switch ki to 2 → esum should be rescaled: 40 * 1 / 2 = 20
    pidComputeOutput(0.0f, esum, elast, Ki_alt, edif, ecount,
                     0.0f, 2.0f, 0.0f, 1000.0f, 1u);
    // After rescaling and one step adding 0*1 = 0: esum ≈ 20
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 20.0f, esum);
}

// --- entry point -------------------------------------------------------------

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_proportional_positive_error);
    RUN_TEST(test_proportional_zero_error);
    RUN_TEST(test_output_clamp_max);
    RUN_TEST(test_output_clamp_min);
    RUN_TEST(test_integral_accumulates);
    RUN_TEST(test_integral_windup_clamp);
    RUN_TEST(test_integral_reset_on_ki_zero);
    RUN_TEST(test_derivative_on_error_change);
    RUN_TEST(test_ki_change_rescales_esum);
    return UNITY_END();
}
