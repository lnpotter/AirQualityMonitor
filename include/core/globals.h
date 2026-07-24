#ifndef GLOBALS_H
#define GLOBALS_H

/** @name Pollutant alert thresholds
 *  Readings above these values are flagged as alerts (see alert_system.c
 *  and export_to_csv.c). Loaded from/saved to the config file by
 *  config_persistence.c; editable at runtime via configure_limits.c.
 *  @{
 */
extern float limit_pm25;
extern float limit_pm10;
extern float limit_co;
extern float limit_no2;
extern float limit_o3;
extern float limit_so2;
/** @} */

/** @brief Interval between simulated/collected readings, in seconds. */
extern int collection_interval;

/** @brief Number of days of readings to keep before cleanup removes them. */
extern int retention_period;

/**
 * @brief One air quality reading: pollutant values plus sensor/timestamp metadata.
 */
typedef struct {
    /** Numeric sensor identifier, matches SensorPlugin::sensor_id. */
    int sensor_id;
    /** Reading timestamp, formatted "%Y-%m-%d %H:%M:%S". */
    char timestamp[32];
    /** Sensor model name, e.g. "PMS5003". */
    char model[64];
    float pm25;
    float pm10;
    float co;
    float no2;
    float o3;
    float so2;
} AirQualityData;

/** @brief Maximum number of sensors that can be configured simultaneously. */
#define MAX_SENSORS 8

/**
 * @brief Configuration for one sensor slot in multi-sensor mode.
 */
typedef struct {
    /** Sensor mode identifier, e.g. "dht22" (see sensor_utils.h). */
    char mode[32];
    /** Path to the plugin library; empty string means use the default
     *  resolved by resolve_default_plugin_path(mode). */
    char plugin_path[256];
    /** Non-zero if this sensor slot is active for data collection. */
    int enabled;
} SensorConfig;

/** @brief Global switch: whether dynamic sensor plugins are used at all. */
extern int sensor_plugins_enabled;
/** @brief Legacy single-sensor mode identifier (pre multi-sensor support). */
extern char sensor_mode[32];
/** @brief Legacy single-sensor plugin path (pre multi-sensor support). */
extern char sensor_plugin_path[256];
/** @brief Multi-sensor configuration slots; only the first active_sensor_count are in use. */
extern SensorConfig sensor_configs[MAX_SENSORS];
/** @brief Number of populated entries in sensor_configs (0..MAX_SENSORS). */
extern int active_sensor_count;

/** @brief Reset sensor_configs and active_sensor_count to their initial (empty) state. */
void init_sensor_configs(void);

#endif
