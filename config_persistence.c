#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONFIG_FILE "config.cfg"

void save_config();
int load_config();

void save_config() {
    FILE *file = fopen(CONFIG_FILE, "w");
    if (file == NULL) {
        fprintf(stderr, "Cannot open config file for writing.\n");
        return;
    }

    fprintf(file, "limit_pm25=%.2f\n", limit_pm25);
    fprintf(file, "limit_pm10=%.2f\n", limit_pm10);
    fprintf(file, "limit_co=%.2f\n", limit_co);
    fprintf(file, "limit_no2=%.2f\n", limit_no2);
    fprintf(file, "limit_o3=%.2f\n", limit_o3);
    fprintf(file, "limit_so2=%.2f\n", limit_so2);
    fprintf(file, "collection_interval=%d\n", collection_interval);
    fprintf(file, "retention_period=%d\n", retention_period);

    fclose(file);
    printf("Configuration saved.\n");
}

int load_config() {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file == NULL) {
        // File does not exist, return 0 to indicate failure
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");

        if (strcmp(key, "limit_pm25") == 0) {
            limit_pm25 = atof(value);
        } else if (strcmp(key, "limit_pm10") == 0) {
            limit_pm10 = atof(value);
        } else if (strcmp(key, "limit_co") == 0) {
            limit_co = atof(value);
        } else if (strcmp(key, "limit_no2") == 0) {
            limit_no2 = atof(value);
        } else if (strcmp(key, "limit_o3") == 0) {
            limit_o3 = atof(value);
        } else if (strcmp(key, "limit_so2") == 0) {
            limit_so2 = atof(value);
        } else if (strcmp(key, "collection_interval") == 0) {
            collection_interval = atoi(value);
        } else if (strcmp(key, "retention_period") == 0) {
            retention_period = atoi(value);
        }
    }

    fclose(file);
    return 1; // Return 1 to indicate success
}
