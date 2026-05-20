#ifndef SENSOR_UTILS_H
#define SENSOR_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Resolve a default plugin library path for a known sensor mode.
 * Returns a static string or NULL if the mode is not recognized.
 */
const char *resolve_default_plugin_path(const char *mode);

/**
 * Returns non-zero if the given sensor mode is recognized.
 */
int is_valid_sensor_mode(const char *mode);

#ifdef __cplusplus
}
#endif

#endif
