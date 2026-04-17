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

static int pms5003_init(void) {
    printf("PMS5003 plugin initialized (simulated readings).\n");
    return 0;
}

static int pms5003_read_sample(AirQualityData *out) {
    if (!out)
        return -1;
    memset(out, 0, sizeof(*out));
    out->sensor_id = 5003;
    fill_timestamp(out->timestamp, sizeof(out->timestamp));

    // Simulated particulate readings.
    out->pm25 = (float)(rand() % 5000) / 100.0f;          // 0..49.99
    out->pm10 = out->pm25 + (float)(rand() % 7000) / 100.0f; // PM10 >= PM2.5
    out->co = 0.0f;
    out->no2 = 0.0f;
    out->o3 = 0.0f;
    out->so2 = 0.0f;
    return 0;
}

static int pms5003_shutdown(void) {
    printf("PMS5003 plugin shutdown.\n");
    return 0;
}

SENSOR_PLUGIN_EXPORT SensorPlugin sensor_plugin = {
    .api_version = SENSOR_PLUGIN_API_VERSION,
    .name = "PMS5003",
    .plugin_version = "1.0.0",
    .description = "Simulated PMS5003 particulate readings.",
    .sensor_id = 5003,
    .init = pms5003_init,
    .read_sample = pms5003_read_sample,
    .shutdown = pms5003_shutdown,
};

