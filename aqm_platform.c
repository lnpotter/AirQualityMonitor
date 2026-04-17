#include "aqm_platform.h"
#include <ctype.h>
#include <errno.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#else
#include <unistd.h>
#endif

int aqm_mkdir_p(const char *path) {
    if (!path || !path[0])
        return -1;
#ifdef _WIN32
    if (_mkdir(path) != 0) {
        DWORD err = GetLastError();
        if (err != ERROR_ALREADY_EXISTS)
            return -1;
    }
    return 0;
#else
    if (mkdir(path, 0755) != 0) {
        if (errno != EEXIST)
            return -1;
    }
    return 0;
#endif
}

void aqm_sleep_seconds(unsigned seconds) {
#ifdef _WIN32
    Sleep((DWORD)seconds * 1000U);
#else
    sleep(seconds);
#endif
}

void aqm_flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void aqm_trim_crlf(char *s) {
    if (!s)
        return;
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' || isspace((unsigned char)s[n - 1]))) {
        s[--n] = '\0';
    }
}
