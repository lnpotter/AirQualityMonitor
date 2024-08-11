#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sqlite3.h>
#include <unistd.h>

#define DB_NAME "air_quality.db"

void generate_random_data(AirQualityData *data);
void insert_data(AirQualityData data);

void interval_collection() {
    AirQualityData data;
    generate_random_data(&data);
    insert_data(data);
    printf("Data collection completed.\n");
}

void generate_random_data(AirQualityData *data) {
    data->sensor_id = rand() % 10 + 1;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf(data->timestamp, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    data->pm25 = (rand() % 100) / 10.0;
    data->pm10 = (rand() % 100) / 10.0;
    data->co = (rand() % 100) / 10.0;
    data->no2 = (rand() % 100) / 1000.0;
    data->o3 = (rand() % 100) / 1000.0;
    data->so2 = (rand() % 100) / 1000.0;
}
