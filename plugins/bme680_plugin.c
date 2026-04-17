#include "../include/sensor/sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef _WIN32
#include <dirent.h>
#endif

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
#ifndef _WIN32
    printf("BME680 plugin initialized (real IIO/sysfs on Linux if available; fallback mock).\n");
#else
    printf("BME680 plugin initialized (mock on Windows).\n");
#endif
    return 0;
}

#ifndef _WIN32
static int read_float_file(const char *path, float *out) {
    FILE *f = fopen(path, "r");
    if (!f)
        return -1;
    float v = 0.0f;
    int ok = fscanf(f, "%f", &v);
    fclose(f);
    if (ok != 1)
        return -1;
    *out = v;
    return 0;
}

static int try_read_bme680_iio(float *temp_c, float *hum_pct, float *gas_kohm) {
    const char *forced = getenv("BME680_IIO_PATH");
    if (forced && forced[0]) {
        char path[512];
        float t = 0.0f, h = 0.0f, g = 0.0f;
        snprintf(path, sizeof(path), "%s/in_temp_input", forced);
        if (read_float_file(path, &t) != 0)
            return -1;
        snprintf(path, sizeof(path), "%s/in_humidityrelative_input", forced);
        if (read_float_file(path, &h) != 0)
            return -1;
        snprintf(path, sizeof(path), "%s/in_resistance_input", forced);
        if (read_float_file(path, &g) != 0)
            g = 0.0f;
        *temp_c = t / 1000.0f;
        *hum_pct = h / 1000.0f;
        *gas_kohm = g / 1000.0f;
        return 0;
    }

    DIR *d = opendir("/sys/bus/iio/devices");
    if (!d)
        return -1;
    struct dirent *ent;
    while ((ent = readdir(d)) != NULL) {
        if (strncmp(ent->d_name, "iio:device", 10) != 0)
            continue;
        char base[512], namef[640], name[128];
        snprintf(base, sizeof(base), "/sys/bus/iio/devices/%s", ent->d_name);
        snprintf(namef, sizeof(namef), "%s/name", base);
        FILE *f = fopen(namef, "r");
        if (!f)
            continue;
        name[0] = '\0';
        fgets(name, sizeof(name), f);
        fclose(f);
        if (strstr(name, "bme680") == NULL && strstr(name, "BME680") == NULL)
            continue;

        float t = 0.0f, h = 0.0f, g = 0.0f;
        char p[640];
        snprintf(p, sizeof(p), "%s/in_temp_input", base);
        if (read_float_file(p, &t) != 0)
            continue;
        snprintf(p, sizeof(p), "%s/in_humidityrelative_input", base);
        if (read_float_file(p, &h) != 0)
            continue;
        snprintf(p, sizeof(p), "%s/in_resistance_input", base);
        if (read_float_file(p, &g) != 0)
            g = 0.0f;
        *temp_c = t / 1000.0f;
        *hum_pct = h / 1000.0f;
        *gas_kohm = g / 1000.0f;
        closedir(d);
        return 0;
    }
    closedir(d);
    return -1;
}
#endif

static int bme680_read_sample(AirQualityData *out) {
    if (!out)
        return -1;
    memset(out, 0, sizeof(*out));
    out->sensor_id = 680;
    snprintf(out->model, sizeof(out->model), "BME680");
    fill_timestamp(out->timestamp, sizeof(out->timestamp));

    // Mapping: pm25=temperature(C), pm10=humidity(%), co=gas resistance(kOhm).
    out->pm25 = 20.0f + (float)(rand() % 1200) / 100.0f;  // 20..31.99 C
    out->pm10 = 35.0f + (float)(rand() % 5000) / 100.0f;  // 35..84.99 %
    out->co = 0.5f + (float)(rand() % 400) / 100.0f;      // 0.5..4.49 proxy
#ifndef _WIN32
    float rt = 0.0f, rh = 0.0f, rg = 0.0f;
    if (try_read_bme680_iio(&rt, &rh, &rg) == 0) {
        out->pm25 = rt;
        out->pm10 = rh;
        if (rg > 0.0f)
            out->co = rg;
    }
#endif
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

