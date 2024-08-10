#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void configure_limits() {
    printf("Please configure the pollutant limits and collection settings:\n");

    char input[10];

    printf("PM2.5 limit (current: %.2f): ", limit_pm25);
    fgets(input, sizeof(input), stdin);
    if (strlen(input) > 1) limit_pm25 = atof(input);

    printf("PM10 limit (current: %.2f): ", limit_pm10);
    fgets(input, sizeof(input), stdin);
    if (strlen(input) > 1) limit_pm10 = atof(input);

    printf("CO limit (current: %.2f): ", limit_co);
    fgets(input, sizeof(input), stdin);
    if (strlen(input) > 1) limit_co = atof(input);

    printf("NO2 limit (current: %.2f): ", limit_no2);
    fgets(input, sizeof(input), stdin);
    if (strlen(input) > 1) limit_no2 = atof(input);

    printf("O3 limit (current: %.2f): ", limit_o3);
    fgets(input, sizeof(input), stdin);
    if (strlen(input) > 1) limit_o3 = atof(input);

    printf("SO2 limit (current: %.2f): ", limit_so2);
    fgets(input, sizeof(input), stdin);
    if (strlen(input) > 1) limit_so2 = atof(input);

    printf("Data collection interval (in seconds, current: %d): ", collection_interval);
    fgets(input, sizeof(input), stdin);
    if (strlen(input) > 1) collection_interval = atoi(input);

    printf("Data retention period (in days, current: %d): ", retention_period);
    fgets(input, sizeof(input), stdin);
    if (strlen(input) > 1) retention_period = atoi(input);

    printf("Configuration updated.\n");
}
