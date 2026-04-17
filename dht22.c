#include "sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static float read_data() {
    float temp = 20.0 + (rand() % 1000) / 100.0;
    printf("DHT22: Temperature = %.2f °C\n", temp);
    return temp;
}

static int init() {
    srand(time(NULL));
    printf("DHT22 initialized.\n");
    return 0;
}

static int shutdown() {
    printf("DHT22 shutdown.\n");
    return 0;
}

Sensor sensor = {
    .name = "DHT22",
    .read_data = read_data,
    .init = init,
    .shutdown = shutdown
};