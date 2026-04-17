#include "core/globals.h"
#include "sensor/sensor_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void show_sensor_runtime_info(void) {
    const char *mode = sensor_mode[0] ? sensor_mode : "mock";
    const char *plugin_path = sensor_plugin_path[0] ? sensor_plugin_path : resolve_default_plugin_path(mode);

    printf("Sensor runtime configuration:\n");
    printf("  Sensor mode=%s\n", mode);
    printf("  Plugins enabled=%s\n", sensor_plugins_enabled ? "yes" : "no");
    printf("  Plugin path=%s\n", (plugin_path && plugin_path[0]) ? plugin_path : "(not set)");

    if (!sensor_plugins_enabled || !plugin_path || !plugin_path[0]) {
        printf("  Active source: builtin mock\n");
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

