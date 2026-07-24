#ifndef SENSOR_UTILS_H
#define SENSOR_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Resolve the default plugin library path for a known sensor mode.
 * @param mode Sensor mode identifier, e.g. "dht22", "pms5003" (case-sensitive).
 * @return A static string with the default plugin path, or NULL if
 *         the mode is not recognized.
 */
const char *resolve_default_plugin_path(const char *mode);

/**
 * @brief Check whether a sensor mode string is one of the known modes.
 * @param mode Sensor mode identifier to check (case-sensitive). NULL
 *        and empty strings are always rejected.
 * @return Non-zero if recognized, 0 otherwise.
 */
int is_valid_sensor_mode(const char *mode);

#ifdef __cplusplus
}
#endif

#endif
