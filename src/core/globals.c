#include "core/globals.h"

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
