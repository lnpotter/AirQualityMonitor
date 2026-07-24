#include "unity.h"
#include "core/aqm_platform.h"
#include <string.h>

void setUp(void) {}
void tearDown(void) {}

void test_parse_int_valid_positive(void) {
    int out = -1;
    TEST_ASSERT_TRUE(aqm_parse_int("42", &out));
    TEST_ASSERT_EQUAL_INT(42, out);
}

void test_parse_int_valid_negative(void) {
    int out = 0;
    TEST_ASSERT_TRUE(aqm_parse_int("-7", &out));
    TEST_ASSERT_EQUAL_INT(-7, out);
}

void test_parse_int_valid_zero(void) {
    int out = -1;
    TEST_ASSERT_TRUE(aqm_parse_int("0", &out));
    TEST_ASSERT_EQUAL_INT(0, out);
}

void test_parse_int_trailing_whitespace_is_accepted(void) {
    int out = -1;
    TEST_ASSERT_TRUE(aqm_parse_int("42 ", &out));
    TEST_ASSERT_EQUAL_INT(42, out);
}

void test_parse_int_rejects_trailing_garbage(void) {
    int out = -1;
    TEST_ASSERT_FALSE(aqm_parse_int("42abc", &out));
}

void test_parse_int_rejects_empty_string(void) {
    int out = -1;
    TEST_ASSERT_FALSE(aqm_parse_int("", &out));
}

void test_parse_int_rejects_non_numeric(void) {
    int out = -1;
    TEST_ASSERT_FALSE(aqm_parse_int("abc", &out));
}

void test_parse_int_rejects_null_input(void) {
    int out = -1;
    TEST_ASSERT_FALSE(aqm_parse_int(NULL, &out));
}

void test_parse_int_rejects_null_output(void) {
    TEST_ASSERT_FALSE(aqm_parse_int("42", NULL));
}

void test_parse_int_leading_whitespace_is_accepted(void) {
    // strtol() itself skips leading whitespace per the C standard
    int out = -1;
    TEST_ASSERT_TRUE(aqm_parse_int("  42", &out));
    TEST_ASSERT_EQUAL_INT(42, out);
}

void test_parse_float_valid_positive(void) {
    float out = -1.0f;
    TEST_ASSERT_TRUE(aqm_parse_float("35.5", &out));
    TEST_ASSERT_EQUAL_FLOAT(35.5f, out);
}

void test_parse_float_valid_negative(void) {
    float out = 0.0f;
    TEST_ASSERT_TRUE(aqm_parse_float("-2.75", &out));
    TEST_ASSERT_EQUAL_FLOAT(-2.75f, out);
}

void test_parse_float_valid_integer_like(void) {
    float out = -1.0f;
    TEST_ASSERT_TRUE(aqm_parse_float("10", &out));
    TEST_ASSERT_EQUAL_FLOAT(10.0f, out);
}

void test_parse_float_rejects_trailing_garbage(void) {
    float out = -1.0f;
    TEST_ASSERT_FALSE(aqm_parse_float("35.5ppm", &out));
}

void test_parse_float_rejects_empty_string(void) {
    float out = -1.0f;
    TEST_ASSERT_FALSE(aqm_parse_float("", &out));
}

void test_parse_float_rejects_non_numeric(void) {
    float out = -1.0f;
    TEST_ASSERT_FALSE(aqm_parse_float("N/A", &out));
}

void test_parse_float_rejects_null_input(void) {
    float out = -1.0f;
    TEST_ASSERT_FALSE(aqm_parse_float(NULL, &out));
}

void test_parse_float_rejects_null_output(void) {
    TEST_ASSERT_FALSE(aqm_parse_float("35.5", NULL));
}

void test_trim_crlf_removes_trailing_newline(void) {
    char buf[32];
    strcpy(buf, "hello\n");
    aqm_trim_crlf(buf);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
}

void test_trim_crlf_removes_trailing_crlf(void) {
    char buf[32];
    strcpy(buf, "hello\r\n");
    aqm_trim_crlf(buf);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
}

void test_trim_crlf_removes_trailing_whitespace(void) {
    // aqm_trim_crlf trims any trailing whitespace, not just \r\n
    char buf[32];
    strcpy(buf, "hello   ");
    aqm_trim_crlf(buf);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
}

void test_trim_crlf_leaves_clean_string_unchanged(void) {
    char buf[32];
    strcpy(buf, "hello");
    aqm_trim_crlf(buf);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
}

void test_trim_crlf_handles_empty_string(void) {
    char buf[32];
    strcpy(buf, "");
    aqm_trim_crlf(buf);
    TEST_ASSERT_EQUAL_STRING("", buf);
}

void test_trim_crlf_handles_null_safely(void) {
    // should not crash
    aqm_trim_crlf(NULL);
    TEST_PASS();
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_parse_int_valid_positive);
    RUN_TEST(test_parse_int_valid_negative);
    RUN_TEST(test_parse_int_valid_zero);
    RUN_TEST(test_parse_int_trailing_whitespace_is_accepted);
    RUN_TEST(test_parse_int_rejects_trailing_garbage);
    RUN_TEST(test_parse_int_rejects_empty_string);
    RUN_TEST(test_parse_int_rejects_non_numeric);
    RUN_TEST(test_parse_int_rejects_null_input);
    RUN_TEST(test_parse_int_rejects_null_output);
    RUN_TEST(test_parse_int_leading_whitespace_is_accepted);

    RUN_TEST(test_parse_float_valid_positive);
    RUN_TEST(test_parse_float_valid_negative);
    RUN_TEST(test_parse_float_valid_integer_like);
    RUN_TEST(test_parse_float_rejects_trailing_garbage);
    RUN_TEST(test_parse_float_rejects_empty_string);
    RUN_TEST(test_parse_float_rejects_non_numeric);
    RUN_TEST(test_parse_float_rejects_null_input);
    RUN_TEST(test_parse_float_rejects_null_output);

    RUN_TEST(test_trim_crlf_removes_trailing_newline);
    RUN_TEST(test_trim_crlf_removes_trailing_crlf);
    RUN_TEST(test_trim_crlf_removes_trailing_whitespace);
    RUN_TEST(test_trim_crlf_leaves_clean_string_unchanged);
    RUN_TEST(test_trim_crlf_handles_empty_string);
    RUN_TEST(test_trim_crlf_handles_null_safely);

    return UNITY_END();
}