#include "aqm_platform.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generate_random_data(AirQualityData *data);
void insert_data(AirQualityData data);

void interval_collection(void) {
    printf("Samples to collect (>=1). Interval between samples: %d s.\n", collection_interval);
    printf("How many samples? ");
    int count = 0;
    if (scanf("%d", &count) != 1 || count < 1) {
        aqm_flush_stdin();
        printf("Invalid count; aborting.\n");
        return;
    }
    aqm_flush_stdin();

    if (collection_interval < 1) {
        printf("collection_interval must be >= 1 (check configuration).\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        AirQualityData data;
        generate_random_data(&data);
        insert_data(data);
        if (i + 1 < count)
            aqm_sleep_seconds((unsigned)collection_interval);
    }

    printf("Data collection finished (%d sample(s)).\n", count);
}

void generate_random_data(AirQualityData *data) {
    data->sensor_id = (rand() % 10) + 1;
    time_t t = time(NULL);
    struct tm *ptm = localtime(&t);
    if (!ptm) {
        snprintf(data->timestamp, sizeof(data->timestamp), "1970-01-01 00:00:00");
        return;
    }
    snprintf(data->timestamp, sizeof(data->timestamp), "%04d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900,
             ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    data->pm25 = (float)((rand() % 1000) / 10.0);
    data->pm10 = (float)((rand() % 1000) / 10.0);
    data->co = (float)((rand() % 1000) / 10.0);
    data->no2 = (float)((rand() % 1000) / 1000.0);
    data->o3 = (float)((rand() % 1000) / 1000.0);
    data->so2 = (float)((rand() % 1000) / 1000.0);
}
