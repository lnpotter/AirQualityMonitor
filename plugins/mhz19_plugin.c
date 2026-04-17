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

static int mhz19_init(void) {
    printf("MH-Z19 plugin initialized (simulated readings).\n");
    return 0;
}

static int mhz19_read_sample(AirQualityData *out) {
    if (!out)
        return -1;
    memset(out, 0, sizeof(*out));
    out->sensor_id = 1900;
    fill_timestamp(out->timestamp, sizeof(out->timestamp));

    // Simulated mapping:
    // co stores CO2 ppm proxy (MH-Z19 primary metric).
    out->pm25 = 0.0f;
    out->pm10 = 0.0f;
    out->co = 400.0f + (float)(rand() % 2600); // 400..2999 ppm proxy
    out->no2 = 0.0f;
    out->o3 = 0.0f;
    out->so2 = 0.0f;
    return 0;
}

static int mhz19_shutdown(void) {
    printf("MH-Z19 plugin shutdown.\n");
    return 0;
}

SENSOR_PLUGIN_EXPORT SensorPlugin sensor_plugin = {
    .api_version = SENSOR_PLUGIN_API_VERSION,
    .name = "MH-Z19",
    .plugin_version = "1.0.0",
    .description = "Simulated MH-Z19 CO2 ppm readings mapped to co field.",
    .sensor_id = 1900,
    .init = mhz19_init,
    .read_sample = mhz19_read_sample,
    .shutdown = mhz19_shutdown,
};

