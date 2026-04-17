#ifndef GLOBALS_H
#define GLOBALS_H

// Pollutant limits
extern float limit_pm25;
extern float limit_pm10;
extern float limit_co;
extern float limit_no2;
extern float limit_o3;
extern float limit_so2;

// Data collection interval (in seconds)
extern int collection_interval;

// Data retention period (in days)
extern int retention_period;

// Structure to hold air quality data
typedef struct {
    int sensor_id;
    char timestamp[32];
    char model[64];
    float pm25;
    float pm10;
    float co;
    float no2;
    float o3;
    float so2;
} AirQualityData;

#define MAX_SENSORS 8

typedef struct {
    char mode[32];
    char plugin_path[256];
    int enabled;
} SensorConfig;

extern int sensor_plugins_enabled;
extern char sensor_mode[32];
extern char sensor_plugin_path[256];
extern SensorConfig sensor_configs[MAX_SENSORS];
extern int active_sensor_count;

void init_sensor_configs(void);

#endif
