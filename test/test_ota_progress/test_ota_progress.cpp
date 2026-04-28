#include <unity.h>
#include "../../src/OtaProgressCalc.h"

void setUp(void) {}
void tearDown(void) {}

// Guard: size == 0 must not crash (was division-by-zero before B48 fix)
void test_progress_size_zero_returns_zero(void)
{
    TEST_ASSERT_EQUAL_UINT8(0u, calcOtaProgress(0u, 0u));
    TEST_ASSERT_EQUAL_UINT8(0u, calcOtaProgress(0u, 100u));
}

void test_progress_zero_done(void)
{
    TEST_ASSERT_EQUAL_UINT8(0u, calcOtaProgress(1000u, 0u));
}

void test_progress_half(void)
{
    TEST_ASSERT_EQUAL_UINT8(50u, calcOtaProgress(1000u, 500u));
}

void test_progress_complete(void)
{
    TEST_ASSERT_EQUAL_UINT8(100u, calcOtaProgress(1000u, 1000u));
}

void test_progress_quarter(void)
{
    TEST_ASSERT_EQUAL_UINT8(25u, calcOtaProgress(1000u, 250u));
}

void test_progress_large_firmware(void)
{
    // Typical 1 MB firmware, 256 kB written
    TEST_ASSERT_EQUAL_UINT8(25u, calcOtaProgress(1048576u, 262144u));
}

void test_progress_clamps_to_100(void)
{
    // done > size (should not happen but must not produce values > 100)
    uint8_t p = calcOtaProgress(100u, 200u);
    TEST_ASSERT_GREATER_OR_EQUAL_UINT8(100u, p);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_progress_size_zero_returns_zero);
    RUN_TEST(test_progress_zero_done);
    RUN_TEST(test_progress_half);
    RUN_TEST(test_progress_complete);
    RUN_TEST(test_progress_quarter);
    RUN_TEST(test_progress_large_firmware);
    RUN_TEST(test_progress_clamps_to_100);
    return UNITY_END();
}
