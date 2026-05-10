#include <unity.h>
#include "../../src/temperature/TemperatureMax31855Calc.h"

void setUp(void) {}
void tearDown(void) {}

// Build raw MAX31855 word: 14-bit temperature field lives in bits [31:18].
static uint32_t makeRaw(uint32_t bits14) {
    return (bits14 & 0x3FFFu) << 18u;
}

// --- fault detection ---------------------------------------------------------

void test_fault_open_circuit(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, MAX31855_INACTIVE_VALUE,
                             calcMax31855Temperature(0x01u));
}

void test_fault_short_to_gnd(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, MAX31855_INACTIVE_VALUE,
                             calcMax31855Temperature(0x02u));
}

void test_fault_short_to_vcc(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, MAX31855_INACTIVE_VALUE,
                             calcMax31855Temperature(0x04u));
}

void test_fault_multiple_bits(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.01f, MAX31855_INACTIVE_VALUE,
                             calcMax31855Temperature(0x07u));
}

// --- positive temperatures ---------------------------------------------------

void test_positive_25_celsius(void) {
    // raw count = (25 + 1.0_adj) / 0.25 = 104
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.0f, calcMax31855Temperature(makeRaw(104u)));
}

void test_positive_100_celsius(void) {
    // raw count = (100 + 1) / 0.25 = 404
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, calcMax31855Temperature(makeRaw(404u)));
}

// --- negative temperatures ---------------------------------------------------

void test_negative_10_celsius(void) {
    // 14-bit two's complement of -36 (magnitude before neg+adj): 2^14 - 36 = 0x3FDC
    // Derivation: need -(count)*0.25 - 1 = -10 → count = 36 → bits14 = ~(36-1) & 0x3FFF = 0x3FDC
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -10.0f, calcMax31855Temperature(makeRaw(0x3FDCu)));
}

void test_negative_1p25_celsius(void) {
    // bits14 = 0x3FFF: sign bit set, all magnitude bits set
    // ~0x3FFF & 0x1FFF = 0 → magnitude+1 = 1 → temperature = -1*0.25 - 1.0 = -1.25 °C
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -1.25f, calcMax31855Temperature(makeRaw(0x3FFFu)));
}

// --- zero / edge cases -------------------------------------------------------

void test_zero_raw_gives_minus_one(void) {
    // rawValue = 0: no fault bits, 14-bit field = 0, positive path
    // temperature = 0 * 0.25 - 1.0 = -1.0 °C
    TEST_ASSERT_FLOAT_WITHIN(0.01f, -1.0f, calcMax31855Temperature(0u));
}

// --- entry point -------------------------------------------------------------

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_fault_open_circuit);
    RUN_TEST(test_fault_short_to_gnd);
    RUN_TEST(test_fault_short_to_vcc);
    RUN_TEST(test_fault_multiple_bits);
    RUN_TEST(test_positive_25_celsius);
    RUN_TEST(test_positive_100_celsius);
    RUN_TEST(test_negative_10_celsius);
    RUN_TEST(test_negative_1p25_celsius);
    RUN_TEST(test_zero_raw_gives_minus_one);
    return UNITY_END();
}
