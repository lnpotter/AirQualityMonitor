#include "sensor/sensor_loader.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

int sensor_module_load(const char *path, SensorModule *out) {
    if (!path || !out || !path[0])
        return -1;

    memset(out, 0, sizeof(*out));

#ifdef _WIN32
    HMODULE h = LoadLibraryA(path);
    if (!h) {
        fprintf(stderr, "Failed to load sensor module: %s\n", path);
        return -1;
    }
    SensorPlugin *plugin = (SensorPlugin *)GetProcAddress(h, "sensor_plugin");
    if (!plugin) {
        fprintf(stderr, "Symbol sensor_plugin not found in %s\n", path);
        FreeLibrary(h);
        return -1;
    }
    out->handle = (void *)h;
    out->plugin = plugin;
#else
    void *h = dlopen(path, RTLD_NOW);
    if (!h) {
        fprintf(stderr, "Failed to load sensor module %s: %s\n", path, dlerror());
        return -1;
    }
    SensorPlugin *plugin = (SensorPlugin *)dlsym(h, "sensor_plugin");
    if (!plugin) {
        fprintf(stderr, "Symbol sensor_plugin not found in %s: %s\n", path, dlerror());
        dlclose(h);
        return -1;
    }
    out->handle = h;
    out->plugin = plugin;
#endif

    if (out->plugin->api_version != SENSOR_PLUGIN_API_VERSION) {
        fprintf(stderr, "Plugin ABI mismatch in %s (got %d, expected %d)\n", path, out->plugin->api_version,
                SENSOR_PLUGIN_API_VERSION);
        sensor_module_unload(out);
        return -1;
    }

    if (!out->plugin->name || !out->plugin->plugin_version || !out->plugin->description || !out->plugin->read_sample ||
        !out->plugin->init || !out->plugin->shutdown) {
        fprintf(stderr, "Invalid plugin ABI in %s\n", path);
        sensor_module_unload(out);
        return -1;
    }

    return 0;
}

void sensor_module_unload(SensorModule *module) {
    if (!module || !module->handle)
        return;

#ifdef _WIN32
    FreeLibrary((HMODULE)module->handle);
#else
    dlclose(module->handle);
#endif
    module->handle = NULL;
    module->plugin = NULL;
}

