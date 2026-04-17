#include "core/globals.h"
#include <string.h>

float limit_pm25 = 35.0f;
float limit_pm10 = 50.0f;
float limit_co = 9.0f;
float limit_no2 = 0.1f;
float limit_o3 = 0.1f;
float limit_so2 = 0.075f;
int collection_interval = 60;
int retention_period = 30;
int sensor_plugins_enabled = 1;
char sensor_mode[32] = "mock";
char sensor_plugin_path[256] = "";
SensorConfig sensor_configs[MAX_SENSORS];
int active_sensor_count = 0;

void init_sensor_configs(void) {
    for (int i = 0; i < MAX_SENSORS; i++) {
        sensor_configs[i].mode[0] = '\0';
        sensor_configs[i].plugin_path[0] = '\0';
        sensor_configs[i].enabled = 0;
    }
    active_sensor_count = 0;
}
