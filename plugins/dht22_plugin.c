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

#if defined(HAVE_WIRINGPI) && !defined(_WIN32)
#include <wiringPi.h>

#ifndef MAX_TIMINGS
#define MAX_TIMINGS 85
#endif

static int dht_pin(void) {
    const char *env = getenv("DHT22_PIN");
    if (!env || !env[0])
        return 7; // wiringPi pin numbering by default
    int v = atoi(env);
    return (v >= 0) ? v : 7;
}

static int dht22_data[5] = {0, 0, 0, 0, 0};

static int dht22_read(float *out_temp_c, float *out_humidity_pct) {
    if (!out_temp_c || !out_humidity_pct)
        return -1;

    const int pin = dht_pin();
    unsigned char laststate = HIGH;
    unsigned char counter = 0;
    unsigned char j = 0;
    unsigned char i;

    dht22_data[0] = dht22_data[1] = dht22_data[2] = dht22_data[3] = dht22_data[4] = 0;

    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    delay(18);
    digitalWrite(pin, HIGH);
    delayMicroseconds(40);
    pinMode(pin, INPUT);

    for (i = 0; i < MAX_TIMINGS; i++) {
        counter = 0;
        while (digitalRead(pin) == laststate) {
            counter++;
            delayMicroseconds(1);
            if (counter == 255)
                break;
        }
        laststate = digitalRead(pin);
        if (counter == 255)
            break;
        if ((i >= 4) && (i % 2 == 0)) {
            dht22_data[j / 8] <<= 1;
            if (counter > 16)
                dht22_data[j / 8] |= 1;
            j++;
        }
    }

    if ((j >= 40) && (dht22_data[4] == ((dht22_data[0] + dht22_data[1] + dht22_data[2] + dht22_data[3]) & 0xFF))) {
        float h = (float)((dht22_data[0] << 8) + dht22_data[1]) / 10.0f;
        if (h > 100)
            h = (float)dht22_data[0];
        float c = (float)(((dht22_data[2] & 0x7F) << 8) + dht22_data[3]) / 10.0f;
        if (c > 125)
            c = (float)dht22_data[2];
        if (dht22_data[2] & 0x80)
            c = -c;

        *out_temp_c = c;
        *out_humidity_pct = h;
        return 0;
    }

    return -1;
}

static int dht22_init(void) {
    if (wiringPiSetup() == -1) {
        fprintf(stderr, "DHT22: wiringPi setup failed\n");
        return -1;
    }
    printf("DHT22 initialized (hardware mode, pin %d).\n", dht_pin());
    return 0;
}

static int dht22_shutdown(void) {
    printf("DHT22 shutdown.\n");
    return 0;
}

#else

// Portable simulated DHT22. Works on Windows/macOS/Linux without extra deps.
static int dht22_read(float *out_temp_c, float *out_humidity_pct) {
    if (!out_temp_c || !out_humidity_pct)
        return -1;

    // Stable-ish values, slightly jittery.
    float temp = 22.0f + (float)(rand() % 800) / 100.0f;      // 22.00 .. 29.99
    float hum = 40.0f + (float)(rand() % 4000) / 100.0f;      // 40.00 .. 79.99
    if (hum > 100.0f)
        hum = 100.0f;

    *out_temp_c = temp;
    *out_humidity_pct = hum;
    return 0;
}

static int dht22_init(void) {
    printf("DHT22 initialized (simulated mode). Set HAVE_WIRINGPI=1 on Linux to use hardware.\n");
    return 0;
}

static int dht22_shutdown(void) {
    printf("DHT22 shutdown.\n");
    return 0;
}

#endif

static int dht22_read_sample(AirQualityData *out) {
    if (!out)
        return -1;

    float t = 0.0f, h = 0.0f;
    if (dht22_read(&t, &h) != 0)
        return -1;

    time_t now = time(NULL);
    struct tm *ptm = localtime(&now);
    if (!ptm)
        return -1;

    memset(out, 0, sizeof(*out));
    out->sensor_id = 22;
    snprintf(out->timestamp, sizeof(out->timestamp), "%04d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900,
             ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    // Current schema doesn't have temperature/humidity fields; map into pm25/pm10 for now.
    out->pm25 = t;
    out->pm10 = h;
    out->co = 0.0f;
    out->no2 = 0.0f;
    out->o3 = 0.0f;
    out->so2 = 0.0f;

    printf("DHT22: Humidity = %.1f %%  Temperature = %.1f C\n", h, t);
    return 0;
}

SENSOR_PLUGIN_EXPORT SensorPlugin sensor_plugin = {
    .api_version = SENSOR_PLUGIN_API_VERSION,
    .name = "DHT22",
    .plugin_version = "1.0.0",
    .description = "DHT22 plugin (hardware with wiringPi or simulated fallback).",
    .sensor_id = 22,
    .init = dht22_init,
    .read_sample = dht22_read_sample,
    .shutdown = dht22_shutdown,
};

