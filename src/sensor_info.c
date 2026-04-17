#include "sensor_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void show_sensor_runtime_info(void) {
    const char *mode = getenv("AQM_SENSOR");
    const char *plugin_path = getenv("AQM_SENSOR_PLUGIN");

    if (!mode || !mode[0])
        mode = "mock";

    printf("Sensor runtime configuration:\n");
    printf("  AQM_SENSOR=%s\n", mode);
    printf("  AQM_SENSOR_PLUGIN=%s\n", (plugin_path && plugin_path[0]) ? plugin_path : "(not set)");

    if (!plugin_path || !plugin_path[0]) {
        printf("  Active source: %s\n", strcmp(mode, "mock") == 0 ? "builtin mock" : "plugin auto-resolution by AQM_SENSOR");
        return;
    }

    SensorModule module;
    if (sensor_module_load(plugin_path, &module) != 0) {
        printf("  Plugin status: failed to load\n");
        return;
    }

    printf("  Plugin status: loaded\n");
    printf("  Plugin name: %s\n", module.plugin->name);
    printf("  Plugin API version: %d\n", module.plugin->api_version);
    printf("  Plugin version: %s\n", module.plugin->plugin_version);
    printf("  Plugin description: %s\n", module.plugin->description);
    printf("  Sensor ID: %d\n", module.plugin->sensor_id);
    sensor_module_unload(&module);
}

