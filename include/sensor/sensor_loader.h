#ifndef SENSOR_LOADER_H
#define SENSOR_LOADER_H

#include "sensor/sensor.h"

typedef struct {
    void *handle;
    const SensorPlugin *plugin;
} SensorModule;

int sensor_module_load(const char *path, SensorModule *out);
void sensor_module_unload(SensorModule *module);

#endif

