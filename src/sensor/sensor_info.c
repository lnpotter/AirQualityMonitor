#include "core/globals.h"
#include "sensor/sensor_loader.h"
#include "sensor/sensor_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void show_single_sensor_info(const char *mode, const char *plugin_path) {
    printf("\n========================================\n");
    printf("  SINGLE SENSOR MODE (Legacy)\n");
    printf("========================================\n");
    printf("  Sensor mode:     %s\n", mode);
    printf("  Plugins enabled: %s\n", sensor_plugins_enabled ? "yes" : "no");
    printf("  Plugin path:     %s\n", (plugin_path && plugin_path[0]) ? plugin_path : "(not set)");

    if (!sensor_plugins_enabled || !plugin_path || !plugin_path[0]) {
        printf("\n  Status: Using builtin mock generator\n");
        return;
    }

    SensorModule module;
    if (sensor_module_load(plugin_path, &module) != 0) {
        printf("\n  Status: FAILED to load plugin\n");
        return;
    }

    printf("\n  Plugin Information:\n");
    printf("  -------------------\n");
    printf("  Name:        %s\n", module.plugin->name ? module.plugin->name : "N/A");
    printf("  Version:     %s\n", module.plugin->plugin_version ? module.plugin->plugin_version : "N/A");
    printf("  API Version: %d\n", module.plugin->api_version);
    printf("  Sensor ID:   %d\n", module.plugin->sensor_id);
    printf("  Description: %s\n", module.plugin->description ? module.plugin->description : "N/A");
    printf("  Status:      Loaded successfully\n");

    sensor_module_unload(&module);
}

static void show_multi_sensor_info(void) {
    printf("\n========================================\n");
    printf("  MULTI-SENSOR MODE\n");
    printf("========================================\n");
    printf("  Total sensors configured: %d (max: %d)\n", active_sensor_count, MAX_SENSORS);
    printf("  Plugins enabled globally: %s\n", sensor_plugins_enabled ? "yes" : "no");

    if (active_sensor_count == 0) {
        printf("\n  No sensors configured. Use option 12 (Auto-detect) or 13 (Configure multi-sensor).\n");
        return;
    }

    printf("\n  Configured Sensors:\n");
    printf("  ===================\n\n");

    for (int i = 0; i < active_sensor_count; i++) {
        SensorConfig *config = &sensor_configs[i];
        const char *plugin_path = config->plugin_path[0] ? config->plugin_path : resolve_default_plugin_path(config->mode);

        printf("  [%d] Sensor: %s\n", i + 1, config->mode);
        printf("       Status:     %s\n", config->enabled ? "ENABLED" : "DISABLED");
        printf("       Plugin:     %s\n", plugin_path ? plugin_path : "(default path)");

        if (sensor_plugins_enabled && plugin_path && plugin_path[0]) {
            SensorModule module;
            if (sensor_module_load(plugin_path, &module) == 0) {
                printf("       Name:       %s\n", module.plugin->name ? module.plugin->name : "N/A");
                printf("       Version:    %s\n", module.plugin->plugin_version ? module.plugin->plugin_version : "N/A");
                printf("       API Ver:    %d\n", module.plugin->api_version);
                printf("       Sensor ID:  %d\n", module.plugin->sensor_id);
                printf("       Load:       OK\n");
                sensor_module_unload(&module);
            } else {
                printf("       Load:       FAILED\n");
            }
        } else {
            printf("       Load:       SKIPPED (plugins disabled)\n");
        }

        if (i < active_sensor_count - 1) {
            printf("\n");
        }
    }

    printf("\n========================================\n");
    printf("  Active for data collection: %d sensor(s)\n", active_sensor_count);
    printf("========================================\n");
}

void show_sensor_runtime_info(void) {
    if (active_sensor_count > 0) {
        show_multi_sensor_info();
    } else {
        const char *mode = sensor_mode[0] ? sensor_mode : "mock";
        const char *plugin_path = sensor_plugin_path[0] ? sensor_plugin_path : resolve_default_plugin_path(mode);
        show_single_sensor_info(mode, plugin_path);
    }
}

