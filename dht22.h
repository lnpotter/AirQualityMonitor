#ifndef DHT22_H
#define DHT22_H

#include "globals.h"

/**
 * DHT22 support
 *
 * - By default (Windows/macOS, or Linux without wiringPi), this compiles in a
 *   portable simulated mode so builds never fail.
 * - To enable the real GPIO implementation on Linux/Raspberry Pi, build with:
 *     make HAVE_WIRINGPI=1
 *   and ensure wiringPi is installed and linkable.
 *
 * Data mapping to the current schema:
 * - temperature (°C) -> pm25
 * - humidity (%)     -> pm10
 * - other fields      -> 0
 * - sensor_id         -> 22 (named \"DHT22\" in DB)
 */

int dht22_init(void);
int dht22_shutdown(void);
int dht22_read(float *out_temp_c, float *out_humidity_pct);
int dht22_read_sample(AirQualityData *out);

#endif

