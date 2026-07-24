#include "unity.h"
#include "sensor/sensor_utils.h"

void setUp(void) {}
void tearDown(void) {}

void test_is_valid_sensor_mode_recognizes_dht22(void) {
    TEST_ASSERT_TRUE(is_valid_sensor_mode("dht22"));
}

void test_is_valid_sensor_mode_recognizes_bme680(void) {
    TEST_ASSERT_TRUE(is_valid_sensor_mode("bme680"));
}

void test_is_valid_sensor_mode_recognizes_pms5003(void) {
    TEST_ASSERT_TRUE(is_valid_sensor_mode("pms5003"));
}

void test_is_valid_sensor_mode_recognizes_mhz19_both_spellings(void) {
    TEST_ASSERT_TRUE(is_valid_sensor_mode("mh-z19"));
    TEST_ASSERT_TRUE(is_valid_sensor_mode("mhz19"));
}

void test_is_valid_sensor_mode_rejects_unknown_mode(void) {
    TEST_ASSERT_FALSE(is_valid_sensor_mode("nonexistent_sensor"));
}

void test_is_valid_sensor_mode_rejects_null(void) {
    TEST_ASSERT_FALSE(is_valid_sensor_mode(NULL));
}

void test_is_valid_sensor_mode_rejects_empty_string(void) {
    TEST_ASSERT_FALSE(is_valid_sensor_mode(""));
}

void test_is_valid_sensor_mode_is_case_sensitive(void) {
    TEST_ASSERT_FALSE(is_valid_sensor_mode("DHT22"));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_is_valid_sensor_mode_recognizes_dht22);
    RUN_TEST(test_is_valid_sensor_mode_recognizes_bme680);
    RUN_TEST(test_is_valid_sensor_mode_recognizes_pms5003);
    RUN_TEST(test_is_valid_sensor_mode_recognizes_mhz19_both_spellings);
    RUN_TEST(test_is_valid_sensor_mode_rejects_unknown_mode);
    RUN_TEST(test_is_valid_sensor_mode_rejects_null);
    RUN_TEST(test_is_valid_sensor_mode_rejects_empty_string);
    RUN_TEST(test_is_valid_sensor_mode_is_case_sensitive);
    return UNITY_END();
}