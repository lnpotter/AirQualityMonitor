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
    
    // Save multi-sensor configurations
    fprintf(file, "active_sensor_count=%d\n", active_sensor_count);
    for (int i = 0; i < active_sensor_count; i++) {
        fprintf(file, "sensor_%d_mode=%s\n", i, sensor_configs[i].mode);
        fprintf(file, "sensor_%d_path=%s\n", i, sensor_configs[i].plugin_path);
        fprintf(file, "sensor_%d_enabled=%d\n", i, sensor_configs[i].enabled);
    }

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

    init_sensor_configs();
    
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

        if (strcmp(key, "limit_pm25") == 0) {
            float temp;
            if (aqm_parse_float(value, &temp))
                limit_pm25 = temp;
        } else if (strcmp(key, "limit_pm10") == 0) {
            float temp;
            if (aqm_parse_float(value, &temp))
                limit_pm10 = temp;
        } else if (strcmp(key, "limit_co") == 0) {
            float temp;
            if (aqm_parse_float(value, &temp))
                limit_co = temp;
        } else if (strcmp(key, "limit_no2") == 0) {
            float temp;
            if (aqm_parse_float(value, &temp))
                limit_no2 = temp;
        } else if (strcmp(key, "limit_o3") == 0) {
            float temp;
            if (aqm_parse_float(value, &temp))
                limit_o3 = temp;
        } else if (strcmp(key, "limit_so2") == 0) {
            float temp;
            if (aqm_parse_float(value, &temp))
                limit_so2 = temp;
        } else if (strcmp(key, "collection_interval") == 0) {
            int temp;
            if (aqm_parse_int(value, &temp) && temp >= 1)
                collection_interval = temp;
        } else if (strcmp(key, "retention_period") == 0) {
            int temp;
            if (aqm_parse_int(value, &temp) && temp >= 1)
                retention_period = temp;
        } else if (strcmp(key, "sensor_plugins_enabled") == 0) {
            int temp;
            if (aqm_parse_int(value, &temp))
                sensor_plugins_enabled = temp ? 1 : 0;
        } else if (strcmp(key, "sensor_mode") == 0)
            snprintf(sensor_mode, sizeof(sensor_mode), "%s", value);
        else if (strcmp(key, "sensor_plugin_path") == 0)
            snprintf(sensor_plugin_path, sizeof(sensor_plugin_path), "%s", value);
        else if (strcmp(key, "active_sensor_count") == 0) {
            int temp;
            if (aqm_parse_int(value, &temp) && temp >= 0 && temp <= MAX_SENSORS)
                active_sensor_count = temp;
        } else if (strncmp(key, "sensor_", 7) == 0) {
            // Parse sensor_N_mode, sensor_N_path, sensor_N_enabled
            int idx = -1;
            char suffix[32] = {0};
            if (sscanf(key, "sensor_%d_%s", &idx, suffix) == 2 && idx >= 0 && idx < MAX_SENSORS) {
                if (strcmp(suffix, "mode") == 0)
                    snprintf(sensor_configs[idx].mode, sizeof(sensor_configs[idx].mode), "%s", value);
                else if (strcmp(suffix, "path") == 0)
                    snprintf(sensor_configs[idx].plugin_path, sizeof(sensor_configs[idx].plugin_path), "%s", value);
                else if (strcmp(suffix, "enabled") == 0)
                    sensor_configs[idx].enabled = atoi(value) ? 1 : 0;
            }
        }
    }

    fclose(file);
    return 1;
}
