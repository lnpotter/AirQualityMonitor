#include "unity.h"
#include <string.h>

int pms5003_parse_frame(const unsigned char frame[32], int *pm25, int *pm10);

void setUp(void) {}
void tearDown(void) {}

// builds a valid 32-byte frame with a correct checksum for the given
// pm25/pm10 raw values (big-endian, as PMS5003 encodes them)
static void build_valid_frame(unsigned char frame[32], int pm25, int pm10) {
    memset(frame, 0, 32);
    frame[0] = 0x42;
    frame[1] = 0x4D;
    frame[12] = (unsigned char)((pm25 >> 8) & 0xFF);
    frame[13] = (unsigned char)(pm25 & 0xFF);
    frame[14] = (unsigned char)((pm10 >> 8) & 0xFF);
    frame[15] = (unsigned char)(pm10 & 0xFF);

    unsigned int sum = 0;
    for (int k = 0; k < 30; k++)
        sum += frame[k];
    frame[30] = (unsigned char)((sum >> 8) & 0xFF);
    frame[31] = (unsigned char)(sum & 0xFF);
}

void test_parse_frame_valid_checksum_extracts_values(void) {
    unsigned char frame[32];
    build_valid_frame(frame, 123, 456);

    int pm25 = -1, pm10 = -1;
    int result = pms5003_parse_frame(frame, &pm25, &pm10);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(123, pm25);
    TEST_ASSERT_EQUAL_INT(456, pm10);
}

void test_parse_frame_zero_values_valid_checksum(void) {
    unsigned char frame[32];
    build_valid_frame(frame, 0, 0);

    int pm25 = -1, pm10 = -1;
    int result = pms5003_parse_frame(frame, &pm25, &pm10);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(0, pm25);
    TEST_ASSERT_EQUAL_INT(0, pm10);
}

void test_parse_frame_max_values_valid_checksum(void) {
    // 16-bit fields: max representable value is 65535
    unsigned char frame[32];
    build_valid_frame(frame, 65535, 65535);

    int pm25 = -1, pm10 = -1;
    int result = pms5003_parse_frame(frame, &pm25, &pm10);

    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_INT(65535, pm25);
    TEST_ASSERT_EQUAL_INT(65535, pm10);
}

void test_parse_frame_corrupted_checksum_is_rejected(void) {
    unsigned char frame[32];
    build_valid_frame(frame, 123, 456);
    frame[31] ^= 0xFF; // flip bits in the low checksum byte -> now invalid

    int pm25 = -1, pm10 = -1;
    int result = pms5003_parse_frame(frame, &pm25, &pm10);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_parse_frame_corrupted_payload_is_rejected(void) {
    unsigned char frame[32];
    build_valid_frame(frame, 123, 456);
    frame[12] ^= 0xFF; // corrupt a payload byte without fixing the checksum

    int pm25 = -1, pm10 = -1;
    int result = pms5003_parse_frame(frame, &pm25, &pm10);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_parse_frame_rejects_null_frame(void) {
    int pm25 = -1, pm10 = -1;
    int result = pms5003_parse_frame(NULL, &pm25, &pm10);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_parse_frame_rejects_null_output_pointers(void) {
    unsigned char frame[32];
    build_valid_frame(frame, 123, 456);

    int pm25 = -1, pm10 = -1;
    TEST_ASSERT_EQUAL_INT(-1, pms5003_parse_frame(frame, NULL, &pm10));
    TEST_ASSERT_EQUAL_INT(-1, pms5003_parse_frame(frame, &pm25, NULL));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse_frame_valid_checksum_extracts_values);
    RUN_TEST(test_parse_frame_zero_values_valid_checksum);
    RUN_TEST(test_parse_frame_max_values_valid_checksum);
    RUN_TEST(test_parse_frame_corrupted_checksum_is_rejected);
    RUN_TEST(test_parse_frame_corrupted_payload_is_rejected);
    RUN_TEST(test_parse_frame_rejects_null_frame);
    RUN_TEST(test_parse_frame_rejects_null_output_pointers);
    return UNITY_END();
}