#ifndef SENSOR_LOADER_H
#define SENSOR_LOADER_H

#include "sensor/sensor.h"

/**
 * @brief A dynamically loaded sensor plugin and its OS-level handle.
 *
 * `handle` is opaque (dlopen()/LoadLibrary() result) and must be passed
 * to sensor_module_unload() to release the library. `plugin` points into
 * the loaded library's memory and becomes invalid after unload.
 */
typedef struct {
    void *handle;
    const SensorPlugin *plugin;
} SensorModule;

/**
 * @brief Load a sensor plugin shared library and validate its ABI.
 * @param path Filesystem path to the .so/.dylib/.dll.
 * @param out  Populated with the handle and plugin pointer on success.
 * @return 0 on success; non-zero if the file can't be loaded, the
 *         `sensor_plugin` symbol is missing, api_version doesn't match
 *         SENSOR_PLUGIN_API_VERSION, or a required function pointer is NULL.
 */
int sensor_module_load(const char *path, SensorModule *out);

/** @brief Unload a previously loaded plugin and release its handle. */
void sensor_module_unload(SensorModule *module);

#endif
