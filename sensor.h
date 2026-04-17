#ifndef SENSOR_H
#define SENSOR_H

#include "globals.h"

typedef struct {
    char name[50];
    float (*read_data)();
    int (*init)();
    int (*shutdown)();
} Sensor;

typedef struct {
    const char *name;
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
