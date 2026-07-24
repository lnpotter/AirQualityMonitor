#ifndef SENSOR_DETECTOR_H
#define SENSOR_DETECTOR_H

#include "sensor/sensor_loader.h"

/**
 * @brief Result of probing one sensor mode during auto-detection.
 */
typedef struct {
    /** Sensor mode identifier, e.g. "dht22", "pms5003". */
    char mode[32];
    /** Resolved plugin library path that was probed. */
    char plugin_path[256];
    /** Plugin-reported name, populated only if detection succeeded. */
    char plugin_name[64];
    /** Non-zero if the plugin library was found and loaded. */
    int detected;
    /** Non-zero if the loaded plugin's init() also succeeded. */
    int can_initialize;
} DetectedSensor;

/**
 * @brief Probe all known sensor modes and report which are usable.
 * @param detected_sensors Output array, populated with one entry per
 *        known sensor mode (regardless of detection outcome).
 * @param max_sensors Capacity of detected_sensors.
 * @return Number of entries written to detected_sensors.
 */
int detect_available_sensors(DetectedSensor *detected_sensors, int max_sensors);

/**
 * @brief Run detect_available_sensors() and interactively prompt the
 *        user to select/configure a sensor from the results.
 */
void run_auto_detection_and_prompt(void);

#endif
