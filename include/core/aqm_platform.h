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

/** Create a single directory (0755 on Unix). Returns 0 on success. */
int aqm_mkdir_p(const char *path);

/** Sleep for whole seconds (cross-platform). */
void aqm_sleep_seconds(unsigned seconds);

/** Discard pending stdin up to newline (call after scanf). */
void aqm_flush_stdin(void);

/** Trim trailing \\r \\n and spaces in place. */
void aqm_trim_crlf(char *s);
/** Parse an integer from user input. Returns 1 if valid. */
int aqm_parse_int(const char *input, int *out);

/** Parse a float from user input. Returns 1 if valid. */
int aqm_parse_float(const char *input, float *out);
#endif
