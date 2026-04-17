#ifndef SENSOR_H
#define SENSOR_H

#include "globals.h"

typedef struct {
    char name[50];
    float (*read_data)();
    int (*init)();
    int (*shutdown)();
} Sensor;

#define SENSOR_PLUGIN_API_VERSION 1

typedef struct {
    int api_version;
    const char *name;
    const char *plugin_version;
    const char *description;
    int sensor_id;
    int (*init)(void);
    int (*read_sample)(AirQualityData *out);
    int (*shutdown)(void);
} SensorPlugin;

/*
 * Dynamic modules must export:
 *   SensorPlugin sensor_plugin;
 * with the exact symbol name above.
 */

#endif
