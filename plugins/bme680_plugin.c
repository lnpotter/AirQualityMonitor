#include "../include/sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define SENSOR_PLUGIN_EXPORT __declspec(dllexport)
#else
#define SENSOR_PLUGIN_EXPORT
#endif

static void fill_timestamp(char *buf, size_t len) {
    time_t now = time(NULL);
    struct tm *ptm = localtime(&now);
    if (!ptm) {
        snprintf(buf, len, "1970-01-01 00:00:00");
        return;
    }
    snprintf(buf, len, "%04d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
             ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}

static int bme680_init(void) {
    printf("BME680 plugin initialized (simulated readings).\n");
    return 0;
}

static int bme680_read_sample(AirQualityData *out) {
    if (!out)
        return -1;
    memset(out, 0, sizeof(*out));
    out->sensor_id = 680;
    fill_timestamp(out->timestamp, sizeof(out->timestamp));

    // Simulated mapping:
    // pm25 = temperature (C), pm10 = humidity (%), co = VOC index proxy.
    out->pm25 = 20.0f + (float)(rand() % 1200) / 100.0f;  // 20..31.99 C
    out->pm10 = 35.0f + (float)(rand() % 5000) / 100.0f;  // 35..84.99 %
    out->co = 0.5f + (float)(rand() % 400) / 100.0f;      // 0.5..4.49 proxy
    out->no2 = 0.0f;
    out->o3 = 0.0f;
    out->so2 = 0.0f;
    return 0;
}

static int bme680_shutdown(void) {
    printf("BME680 plugin shutdown.\n");
    return 0;
}

SENSOR_PLUGIN_EXPORT SensorPlugin sensor_plugin = {
    .api_version = SENSOR_PLUGIN_API_VERSION,
    .name = "BME680",
    .plugin_version = "1.0.0",
    .description = "Simulated BME680 readings (temperature/humidity/VOC proxy).",
    .sensor_id = 680,
    .init = bme680_init,
    .read_sample = bme680_read_sample,
    .shutdown = bme680_shutdown,
};

