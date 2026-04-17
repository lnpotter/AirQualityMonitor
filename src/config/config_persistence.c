#include "core/aqm_paths.h"
#include "core/aqm_platform.h"
#include "core/globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void save_config(void) {
    char path[AQM_PATH_MAX];
    if (aqm_get_config_path(path, sizeof(path)) != 0) {
        fprintf(stderr, "Could not resolve config path.\n");
        return;
    }

    FILE *file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "Cannot open config file for writing: %s\n", path);
        return;
    }

    fprintf(file, "limit_pm25=%.4f\n", limit_pm25);
    fprintf(file, "limit_pm10=%.4f\n", limit_pm10);
    fprintf(file, "limit_co=%.4f\n", limit_co);
    fprintf(file, "limit_no2=%.6f\n", limit_no2);
    fprintf(file, "limit_o3=%.6f\n", limit_o3);
    fprintf(file, "limit_so2=%.6f\n", limit_so2);
    fprintf(file, "collection_interval=%d\n", collection_interval);
    fprintf(file, "retention_period=%d\n", retention_period);
    fprintf(file, "sensor_plugins_enabled=%d\n", sensor_plugins_enabled);
    fprintf(file, "sensor_mode=%s\n", sensor_mode);
    fprintf(file, "sensor_plugin_path=%s\n", sensor_plugin_path);

    fclose(file);
    printf("Configuration saved to %s\n", path);
}

int load_config(void) {
    char path[AQM_PATH_MAX];
    if (aqm_get_config_path(path, sizeof(path)) != 0)
        return 0;

    FILE *file = fopen(path, "r");
    if (file == NULL)
        return 0;

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        aqm_trim_crlf(line);
        if (line[0] == '\0')
            continue;

        char *eq = strchr(line, '=');
        if (!eq)
            continue;
        *eq = '\0';
        const char *key = line;
        const char *value = eq + 1;

        if (strcmp(key, "limit_pm25") == 0)
            limit_pm25 = (float)atof(value);
        else if (strcmp(key, "limit_pm10") == 0)
            limit_pm10 = (float)atof(value);
        else if (strcmp(key, "limit_co") == 0)
            limit_co = (float)atof(value);
        else if (strcmp(key, "limit_no2") == 0)
            limit_no2 = (float)atof(value);
        else if (strcmp(key, "limit_o3") == 0)
            limit_o3 = (float)atof(value);
        else if (strcmp(key, "limit_so2") == 0)
            limit_so2 = (float)atof(value);
        else if (strcmp(key, "collection_interval") == 0)
            collection_interval = atoi(value);
        else if (strcmp(key, "retention_period") == 0)
            retention_period = atoi(value);
        else if (strcmp(key, "sensor_plugins_enabled") == 0)
            sensor_plugins_enabled = atoi(value) ? 1 : 0;
        else if (strcmp(key, "sensor_mode") == 0)
            snprintf(sensor_mode, sizeof(sensor_mode), "%s", value);
        else if (strcmp(key, "sensor_plugin_path") == 0)
            snprintf(sensor_plugin_path, sizeof(sensor_plugin_path), "%s", value);
    }

    fclose(file);
    return 1;
}
