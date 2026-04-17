#ifndef SENSOR_DETECTOR_H
#define SENSOR_DETECTOR_H

#include "sensor/sensor_loader.h"

typedef struct {
    char mode[32];
    char plugin_path[256];
    char plugin_name[64];
    int detected;
    int can_initialize;
} DetectedSensor;

int detect_available_sensors(DetectedSensor *detected_sensors, int max_sensors);
void run_auto_detection_and_prompt(void);

#endif
