#include "aqm_platform.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void configure_limits(void) {
    aqm_flush_stdin();

    printf("Configure pollutant limits and collection settings (press Enter to keep current value):\n");

    char input[64];

    printf("PM2.5 limit (current: %.4f): ", limit_pm25);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1)
        limit_pm25 = (float)atof(input);

    printf("PM10 limit (current: %.4f): ", limit_pm10);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1)
        limit_pm10 = (float)atof(input);

    printf("CO limit (current: %.4f): ", limit_co);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1)
        limit_co = (float)atof(input);

    printf("NO2 limit (current: %.6f): ", limit_no2);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1)
        limit_no2 = (float)atof(input);

    printf("O3 limit (current: %.6f): ", limit_o3);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1)
        limit_o3 = (float)atof(input);

    printf("SO2 limit (current: %.6f): ", limit_so2);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1)
        limit_so2 = (float)atof(input);

    printf("Data collection interval in seconds (current: %d): ", collection_interval);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1)
        collection_interval = atoi(input);

    printf("Data retention period in days (current: %d): ", retention_period);
    if (fgets(input, sizeof(input), stdin) && strlen(input) > 1)
        retention_period = atoi(input);

    if (collection_interval < 1)
        collection_interval = 1;
    if (retention_period < 1)
        retention_period = 1;

    printf("Configuration updated.\n");
}
