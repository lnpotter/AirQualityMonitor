#include "sensor/sensor_utils.h"
#include <string.h>

const char *resolve_default_plugin_path(const char *mode) {
    if (!mode)
        return NULL;
#ifdef _WIN32
    if (strcmp(mode, "dht22") == 0)
        return ".\\plugins\\dht22_plugin.dll";
    if (strcmp(mode, "bme680") == 0)
        return ".\\plugins\\bme680_plugin.dll";
    if (strcmp(mode, "pms5003") == 0)
        return ".\\plugins\\pms5003_plugin.dll";
    if (strcmp(mode, "mh-z19") == 0 || strcmp(mode, "mhz19") == 0)
        return ".\\plugins\\mhz19_plugin.dll";
#elif __APPLE__
    if (strcmp(mode, "dht22") == 0)
        return "./plugins/dht22_plugin.dylib";
    if (strcmp(mode, "bme680") == 0)
        return "./plugins/bme680_plugin.dylib";
    if (strcmp(mode, "pms5003") == 0)
        return "./plugins/pms5003_plugin.dylib";
    if (strcmp(mode, "mh-z19") == 0 || strcmp(mode, "mhz19") == 0)
        return "./plugins/mhz19_plugin.dylib";
#else
    if (strcmp(mode, "dht22") == 0)
        return "./plugins/dht22_plugin.so";
    if (strcmp(mode, "bme680") == 0)
        return "./plugins/bme680_plugin.so";
    if (strcmp(mode, "pms5003") == 0)
        return "./plugins/pms5003_plugin.so";
    if (strcmp(mode, "mh-z19") == 0 || strcmp(mode, "mhz19") == 0)
        return "./plugins/mhz19_plugin.so";
#endif
    return NULL;
}

int is_valid_sensor_mode(const char *mode) {
    return resolve_default_plugin_path(mode) != NULL;
}
