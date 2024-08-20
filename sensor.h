#ifndef SENSOR_H
#define SENSOR_H

typedef struct {
    char name[50];
    float (*read_data)();
    int (*init)();
    int (*shutdown)();
} Sensor;

#endif
