#include "sensor/sensor_detector.h"
#include "core/globals.h"
#include "core/aqm_platform.h"
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

int detect_available_sensors(DetectedSensor *detected_sensors, int max_sensors) {
    const char *sensor_modes[] = {"dht22", "bme680", "pms5003", "mhz19"};
    int count = 0;

    for (size_t i = 0; i < sizeof(sensor_modes) / sizeof(sensor_modes[0]) && count < max_sensors; i++) {
        const char *mode = sensor_modes[i];
        const char *plugin_path = resolve_default_plugin_path(mode);
        
        if (!plugin_path) {
            continue;
        }

        DetectedSensor *ds = &detected_sensors[count];
        memset(ds, 0, sizeof(DetectedSensor));
        snprintf(ds->mode, sizeof(ds->mode), "%s", mode);
        snprintf(ds->plugin_path, sizeof(ds->plugin_path), "%s", plugin_path);
        ds->detected = 0;
        ds->can_initialize = 0;
        ds->plugin_name[0] = '\0';

        SensorModule module;
        if (sensor_module_load(plugin_path, &module) == 0) {
            ds->detected = 1;
            // Copy the plugin name to our buffer before unloading
            if (module.plugin->name) {
                snprintf(ds->plugin_name, sizeof(ds->plugin_name), "%s", module.plugin->name);
            }
            
            // Try to initialize the sensor to check if it's actually available
            if (module.plugin->init() == 0) {
                ds->can_initialize = 1;
                module.plugin->shutdown();
            }
            
            sensor_module_unload(&module);
        }

        count++;
    }

    return count;
}

void run_auto_detection_and_prompt(void) {
    printf("\n=== Auto-Detecting Sensors ===\n");
    printf("Scanning for available sensor plugins...\n\n");

    DetectedSensor detected_sensors[MAX_SENSORS];
    int count = detect_available_sensors(detected_sensors, MAX_SENSORS);

    if (count == 0) {
        printf("No sensor plugins found in the plugins directory.\n");
        return;
    }

    printf("Found %d sensor plugin(s):\n\n", count);

    int available_count = 0;
    for (int i = 0; i < count; i++) {
        printf("%d. %s (%s)\n", i + 1, detected_sensors[i].mode, detected_sensors[i].plugin_path);
        if (detected_sensors[i].detected) {
            printf("   Status: Plugin loaded successfully\n");
            printf("   Name: %s\n", detected_sensors[i].plugin_name[0] ? detected_sensors[i].plugin_name : "Unknown");
            if (detected_sensors[i].can_initialize) {
                printf("   Hardware: Sensor initialized successfully (likely connected)\n");
                available_count++;
            } else {
                printf("   Hardware: Sensor failed to initialize (may not be connected)\n");
            }
        } else {
            printf("   Status: Plugin not found or failed to load\n");
        }
        printf("\n");
    }

    if (available_count == 0) {
        printf("No sensors could be initialized. You can still use mock mode.\n");
        return;
    }

    printf("%d sensor(s) appear to be connected and ready.\n", available_count);
    printf("\nDo you want to enable these sensors for multi-sensor data collection? (y/n): ");
    
    char input[8];
    if (fgets(input, sizeof(input), stdin) && (input[0] == 'y' || input[0] == 'Y')) {
        printf("\nEnabling detected sensors...\n");
        
        init_sensor_configs();
        active_sensor_count = 0;
        
        for (int i = 0; i < count; i++) {
            if (detected_sensors[i].detected && detected_sensors[i].can_initialize) {
                if (active_sensor_count < MAX_SENSORS) {
                    SensorConfig *config = &sensor_configs[active_sensor_count];
                    snprintf(config->mode, sizeof(config->mode), "%s", detected_sensors[i].mode);
                    snprintf(config->plugin_path, sizeof(config->plugin_path), "%s", detected_sensors[i].plugin_path);
                    config->enabled = 1;
                    
                    printf("  - Enabled %s (sensor_id: %d)\n", detected_sensors[i].plugin_name[0] ? detected_sensors[i].plugin_name : detected_sensors[i].mode, active_sensor_count);
                    active_sensor_count++;
                }
            }
        }
        
        if (active_sensor_count > 0) {
            printf("\nSuccessfully enabled %d sensor(s) for multi-sensor data collection.\n", active_sensor_count);
            printf("Configuration will be saved when you exit.\n");
        } else {
            printf("\nNo sensors were enabled.\n");
        }
    } else {
        printf("\nAuto-detection canceled. No changes made.\n");
    }
}
