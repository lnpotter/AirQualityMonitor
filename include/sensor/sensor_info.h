#ifndef SENSOR_INFO_H
#define SENSOR_INFO_H

/**
 * @brief Print the current sensor configuration and runtime status to stdout.
 *
 * Shows multi-sensor details (one entry per configured sensor, each
 * attempting a real plugin load to report name/version/API) when
 * active_sensor_count > 0, otherwise falls back to legacy single-sensor
 * info for the globals-based sensor_mode/sensor_plugin_path.
 */
void show_sensor_runtime_info(void);

#endif
