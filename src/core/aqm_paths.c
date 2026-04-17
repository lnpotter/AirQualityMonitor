#include "core/aqm_paths.h"
#include "core/aqm_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char g_data_dir[AQM_PATH_MAX];

static int append_path(char *buf, size_t buflen, const char *a, const char *b) {
    int n = snprintf(buf, buflen, "%s%s%s", a, AQM_PATH_SEP, b);
    if (n < 0 || (size_t)n >= buflen)
        return -1;
    return 0;
}

void aqm_paths_init(void) {
    const char *env = getenv("AIR_QUALITY_DATA_DIR");
    if (env && env[0] != '\0') {
        size_t len = strlen(env);
        if (len >= sizeof(g_data_dir)) {
            fprintf(stderr, "AIR_QUALITY_DATA_DIR path too long.\n");
            g_data_dir[0] = '\0';
            return;
        }
        memcpy(g_data_dir, env, len + 1);
    } else {
        if (snprintf(g_data_dir, sizeof(g_data_dir), "data") >= (int)sizeof(g_data_dir)) {
            g_data_dir[0] = '\0';
            return;
        }
    }

    if (aqm_mkdir_p(g_data_dir) != 0) {
        fprintf(stderr, "Could not create data directory: %s\n", g_data_dir);
        g_data_dir[0] = '\0';
    }
}

int aqm_get_data_dir(char *buf, size_t buflen) {
    if (!buf || buflen == 0)
        return -1;
    if (g_data_dir[0] == '\0') {
        aqm_paths_init();
    }
    if (g_data_dir[0] == '\0')
        return -1;
    if (strlen(g_data_dir) >= buflen)
        return -1;
    memcpy(buf, g_data_dir, strlen(g_data_dir) + 1);
    return 0;
}

int aqm_get_db_path(char *buf, size_t buflen) {
    char dir[AQM_PATH_MAX];
    if (aqm_get_data_dir(dir, sizeof(dir)) != 0)
        return -1;
    return append_path(buf, buflen, dir, "air_quality.db");
}

int aqm_get_config_path(char *buf, size_t buflen) {
    char dir[AQM_PATH_MAX];
    if (aqm_get_data_dir(dir, sizeof(dir)) != 0)
        return -1;
    return append_path(buf, buflen, dir, "config.cfg");
}
