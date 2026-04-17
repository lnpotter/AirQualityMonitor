#include "aqm_platform.h"
#include "dht22.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void mock_generate_air_quality(AirQualityData *data);
void insert_data(AirQualityData data);

void interval_collection(void) {
    const char *mode = getenv("AQM_SENSOR");
    if (!mode || !mode[0])
        mode = "mock";

    int use_dht22 = strcmp(mode, "dht22") == 0;
    if (use_dht22) {
        if (dht22_init() != 0) {
            fprintf(stderr, "DHT22 init failed; falling back to mock.\n");
            use_dht22 = 0;
        }
    }

    printf("Samples to collect (>=1). Interval between samples: %d s.\n", collection_interval);
    printf("Sensor mode: %s\n", use_dht22 ? "dht22" : "mock");
    printf("How many samples? ");
    int count = 0;
    if (scanf("%d", &count) != 1 || count < 1) {
        aqm_flush_stdin();
        printf("Invalid count; aborting.\n");
        if (use_dht22)
            dht22_shutdown();
        return;
    }
    aqm_flush_stdin();

    if (collection_interval < 1) {
        printf("collection_interval must be >= 1 (check configuration).\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        AirQualityData data;
        if (use_dht22) {
            if (dht22_read_sample(&data) != 0) {
                fprintf(stderr, "DHT22 read failed; using mock sample.\n");
                mock_generate_air_quality(&data);
            }
        } else {
            mock_generate_air_quality(&data);
        }
        insert_data(data);
        if (i + 1 < count)
            aqm_sleep_seconds((unsigned)collection_interval);
    }

    printf("Data collection finished (%d sample(s)).\n", count);
    if (use_dht22)
        dht22_shutdown();
}

static void mock_generate_air_quality(AirQualityData *data) {
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
