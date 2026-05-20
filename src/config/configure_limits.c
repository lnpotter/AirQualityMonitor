#include "core/aqm_platform.h"
#include "core/globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void configure_limits(void) {
    aqm_flush_stdin();

    printf("Configure pollutant limits and collection settings (press Enter to keep current value):\n");

    char input[64];

    printf("PM2.5 limit (current: %.4f): ", limit_pm25);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1) {
        float temp;
        if (aqm_parse_float(input, &temp))
            limit_pm25 = temp;
    }

    printf("PM10 limit (current: %.4f): ", limit_pm10);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1) {
        float temp;
        if (aqm_parse_float(input, &temp))
            limit_pm10 = temp;
    }

    printf("CO limit (current: %.4f): ", limit_co);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1) {
        float temp;
        if (aqm_parse_float(input, &temp))
            limit_co = temp;
    }

    printf("NO2 limit (current: %.6f): ", limit_no2);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1) {
        float temp;
        if (aqm_parse_float(input, &temp))
            limit_no2 = temp;
    }

    printf("O3 limit (current: %.6f): ", limit_o3);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1) {
        float temp;
        if (aqm_parse_float(input, &temp))
            limit_o3 = temp;
    }

    printf("SO2 limit (current: %.6f): ", limit_so2);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1) {
        float temp;
        if (aqm_parse_float(input, &temp))
            limit_so2 = temp;
    }

    printf("Data collection interval in seconds (current: %d): ", collection_interval);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1) {
        int temp;
        if (aqm_parse_int(input, &temp) && temp >= 1)
            collection_interval = temp;
    }

    printf("Data retention period in days (current: %d): ", retention_period);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1) {
        int temp;
        if (aqm_parse_int(input, &temp) && temp >= 1)
            retention_period = temp;
    }

    if (collection_interval < 1)
        collection_interval = 1;
    if (retention_period < 1)
        retention_period = 1;

    printf("Configuration updated.\n");
}
