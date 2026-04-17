#include "core/aqm_platform.h"
#include "core/globals.h"
#include "sensor/sensor_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void mock_generate_air_quality(AirQualityData *data);
static const char *resolve_default_plugin_path(const char *mode);
static void fill_timestamp(char *buf, size_t len);
void insert_data(AirQualityData data);

void interval_collection(void) {
    const char *mode = sensor_mode[0] ? sensor_mode : "mock";
    const char *plugin_path_env = sensor_plugin_path[0] ? sensor_plugin_path : NULL;

    const char *plugin_path = plugin_path_env && plugin_path_env[0] ? plugin_path_env : resolve_default_plugin_path(mode);
    SensorModule module;
    int use_plugin = 0;

    if (sensor_plugins_enabled && plugin_path) {
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

    printf("Samples to collect (>=1). Interval between samples: %d s.\n", collection_interval);
    if (use_plugin)
        printf("Sensor mode: plugin (%s)\n", module.plugin->name);
    else
        printf("Sensor mode: mock\n");
    printf("How many samples? ");
    int count = 0;
    if (scanf("%d", &count) != 1 || count < 1) {
        aqm_flush_stdin();
        printf("Invalid count; aborting.\n");
        if (use_plugin) {
            module.plugin->shutdown();
            sensor_module_unload(&module);
        }
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
            } else if (!data.model[0]) {
                snprintf(data.model, sizeof(data.model), "%s", module.plugin->name);
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
}

static void mock_generate_air_quality(AirQualityData *data) {
    data->sensor_id = (rand() % 10) + 1;
    snprintf(data->model, sizeof(data->model), "mock");
    fill_timestamp(data->timestamp, sizeof(data->timestamp));

    data->pm25 = (float)((rand() % 1000) / 10.0);
    data->pm10 = (float)((rand() % 1000) / 10.0);
    data->co = (float)((rand() % 1000) / 10.0);
    data->no2 = (float)((rand() % 1000) / 1000.0);
    data->o3 = (float)((rand() % 1000) / 1000.0);
    data->so2 = (float)((rand() % 1000) / 1000.0);
}

static void fill_timestamp(char *buf, size_t len) {
    time_t t = time(NULL);
    struct tm *ptm = localtime(&t);
    if (!ptm) {
        snprintf(buf, len, "1970-01-01 00:00:00");
        return;
    }
    if (strftime(buf, len, "%Y-%m-%d %H:%M:%S", ptm) == 0) {
        snprintf(buf, len, "1970-01-01 00:00:00");
    }
}

static const char *resolve_default_plugin_path(const char *mode) {
    if (!mode)
        return NULL;
#ifdef _WIN32
    if (strcmp(mode, "dht22") == 0)
        return ".\\plugins\\dht22_plugin.dll";
    if (strcmp(mode, "bme680") == 0)
        return ".\\plugins\\bme680_plugin.dll";
    if (strcmp(mode, "pms5003") == 0)
        return ".\\plugins\\pms5003_plugin.dll";
    if (strcmp(mode, "mh-z19") == 0 || strcmp(mode, "mhz19") == 0)
        return ".\\plugins\\mhz19_plugin.dll";
#elif __APPLE__
    if (strcmp(mode, "dht22") == 0)
        return "./plugins/dht22_plugin.dylib";
    if (strcmp(mode, "bme680") == 0)
        return "./plugins/bme680_plugin.dylib";
    if (strcmp(mode, "pms5003") == 0)
        return "./plugins/pms5003_plugin.dylib";
    if (strcmp(mode, "mh-z19") == 0 || strcmp(mode, "mhz19") == 0)
        return "./plugins/mhz19_plugin.dylib";
#else
    if (strcmp(mode, "dht22") == 0)
        return "./plugins/dht22_plugin.so";
    if (strcmp(mode, "bme680") == 0)
        return "./plugins/bme680_plugin.so";
    if (strcmp(mode, "pms5003") == 0)
        return "./plugins/pms5003_plugin.so";
    if (strcmp(mode, "mh-z19") == 0 || strcmp(mode, "mhz19") == 0)
        return "./plugins/mhz19_plugin.so";
#endif
    return NULL;
}
