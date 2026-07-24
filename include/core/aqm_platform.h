#ifndef AQM_PLATFORM_H
#define AQM_PLATFORM_H

#include <stdio.h>

#ifdef _WIN32
#define AQM_PATH_SEP "\\"
#else
#define AQM_PATH_SEP "/"
#include <sys/stat.h>
#include <sys/types.h>
#endif

/**
 * @brief Create a single directory (0755 on Unix), cross-platform.
 * @param path Directory path to create. Does not create parent
 *        directories recursively.
 * @return 0 on success (including if the directory already exists),
 *         non-zero on failure.
 */
int aqm_mkdir_p(const char *path);

/** @brief Sleep for a whole number of seconds (cross-platform). */
void aqm_sleep_seconds(unsigned seconds);

/**
 * @brief Discard any pending input up to and including the next
 *        newline. Call after scanf()-family functions that leave a
 *        trailing newline in stdin.
 */
void aqm_flush_stdin(void);

/**
 * @brief Trim trailing `\r`, `\n`, and other whitespace from a string, in place.
 * @param s NUL-terminated string to trim. Safe to call with NULL (no-op).
 */
void aqm_trim_crlf(char *s);

/**
 * @brief Parse a string as an integer, rejecting any trailing
 *        non-whitespace garbage.
 *
 * Leading and trailing whitespace around the number are tolerated
 * (inherited from strtol()); anything else after the number is
 * rejected.
 * @param input NUL-terminated string to parse.
 * @param out Receives the parsed value on success; unchanged on failure.
 * @return 1 if input was a valid integer, 0 otherwise (including
 *         NULL input/out, empty string, or out-of-range value).
 */
int aqm_parse_int(const char *input, int *out);

/**
 * @brief Parse a string as a float, rejecting any trailing
 *        non-whitespace garbage.
 *
 * Leading and trailing whitespace around the number are tolerated
 * (inherited from strtof()); anything else after the number is
 * rejected.
 * @param input NUL-terminated string to parse.
 * @param out Receives the parsed value on success; unchanged on failure.
 * @return 1 if input was a valid float, 0 otherwise (including
 *         NULL input/out or empty string).
 */
int aqm_parse_float(const char *input, float *out);

#endif
