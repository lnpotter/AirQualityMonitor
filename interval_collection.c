#include "aqm_platform.h"
#include "dht22.h"
#include "globals.h"
#include "sensor_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void mock_generate_air_quality(AirQualityData *data);
void insert_data(AirQualityData data);

void interval_collection(void) {
    const char *mode = getenv("AQM_SENSOR");
    const char *plugin_path = getenv("AQM_SENSOR_PLUGIN");
    if (!mode || !mode[0])
        mode = "mock";

    int use_dht22 = strcmp(mode, "dht22") == 0;
    SensorModule module;
    int use_plugin = 0;

    if (plugin_path && plugin_path[0]) {
        if (sensor_module_load(plugin_path, &module) == 0) {
            if (module.plugin->init() == 0) {
                use_plugin = 1;
            } else {
                fprintf(stderr, "Plugin init failed; ignoring plugin: %s\n", plugin_path);
                sensor_module_unload(&module);
            }
        } else {
            fprintf(stderr, "Could not load plugin from %s. Continuing with builtin sensors.\n", plugin_path);
        }
    }

    if (!use_plugin && use_dht22) {
        if (dht22_init() != 0) {
            fprintf(stderr, "DHT22 init failed; falling back to mock.\n");
            use_dht22 = 0;
        }
    }

    printf("Samples to collect (>=1). Interval between samples: %d s.\n", collection_interval);
    if (use_plugin)
        printf("Sensor mode: plugin (%s)\n", module.plugin->name);
    else
        printf("Sensor mode: %s\n", use_dht22 ? "dht22" : "mock");
    printf("How many samples? ");
    int count = 0;
    if (scanf("%d", &count) != 1 || count < 1) {
        aqm_flush_stdin();
        printf("Invalid count; aborting.\n");
        if (use_plugin) {
            module.plugin->shutdown();
            sensor_module_unload(&module);
        }
        if (use_dht22)
            dht22_shutdown();
        return;
    }
    aqm_flush_stdin();

    if (collection_interval < 1) {
        printf("collection_interval must be >= 1 (check configuration).\n");
        if (use_plugin) {
            module.plugin->shutdown();
            sensor_module_unload(&module);
        }
        return;
    }

    for (int i = 0; i < count; i++) {
        AirQualityData data;
        if (use_plugin) {
            if (module.plugin->read_sample(&data) != 0) {
                fprintf(stderr, "Plugin read failed (%s); using mock sample.\n", module.plugin->name);
                mock_generate_air_quality(&data);
            }
        } else if (use_dht22) {
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
    if (use_plugin) {
        module.plugin->shutdown();
        sensor_module_unload(&module);
    }
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
