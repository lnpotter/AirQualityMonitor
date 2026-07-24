#ifndef SENSOR_H
#define SENSOR_H

#include "core/globals.h"

/**
 * @brief Legacy sensor interface, superseded by SensorPlugin.
 *
 * Retained for reference; not used by the current plugin-loading
 * pipeline (see SensorPlugin below).
 */
typedef struct {
    char name[50];
    float (*read_data)();
    int (*init)();
    int (*shutdown)();
} Sensor;

/**
 * @brief ABI version for SensorPlugin. Bump when the struct layout changes.
 *
 * sensor_module_load() rejects any plugin whose api_version does not
 * match this value, preventing a stale/incompatible .so/.dylib/.dll
 * from being loaded silently.
 */
#define SENSOR_PLUGIN_API_VERSION 1

/**
 * @brief Runtime contract every sensor plugin (.so/.dylib/.dll) must implement.
 *
 * A plugin exports a single global symbol named `sensor_plugin` of this
 * type. sensor_module_load() validates api_version and the three
 * required function pointers before the plugin is considered usable.
 */
typedef struct {
    /** Must equal SENSOR_PLUGIN_API_VERSION at build time. */
    int api_version;
    /** Human-readable sensor name, e.g. "PMS5003". */
    const char *name;
    /** Plugin version string, e.g. "1.0.0". Informational only. */
    const char *plugin_version;
    /** Short description shown in sensor info/listing screens. */
    const char *description;
    /** Numeric identifier stored alongside readings in the database. */
    int sensor_id;
    /** Called once after loading. Returns 0 on success. */
    int (*init)(void);
    /**
     * Reads one sample into *out.
     * @return 0 on success; non-zero on failure (e.g. hardware
     *         unavailable) — callers should fall back gracefully.
     */
    int (*read_sample)(AirQualityData *out);
    /** Called once before unloading, for cleanup. Returns 0 on success. */
    int (*shutdown)(void);
} SensorPlugin;

/**
 * @brief Required export contract for dynamic sensor plugins.
 *
 * Every plugin shared library must export a single global symbol:
 * @code
 * SensorPlugin sensor_plugin;
 * @endcode
 * with exactly that name. sensor_module_load() looks it up via
 * dlsym()/GetProcAddress() and validates it before use.
 */

#endif
