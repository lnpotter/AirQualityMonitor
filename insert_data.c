#include "globals.h"
#include <stdio.h>
#include <sqlite3.h>
#include <time.h>

#define DB_NAME "air_quality.db"

typedef struct {
    int sensor_id;
    char timestamp[20];
    float pm25;
    float pm10;
    float co;
    float no2;
    float o3;
    float so2;
} AirQualityData;

void generate_random_data(AirQualityData *data);
void insert_data(AirQualityData data);

void insert_data(AirQualityData data) {
    sqlite3 *db;
    char *err_msg = 0;

    int rc = sqlite3_open(DB_NAME, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char sql[256];
    sprintf(sql, "INSERT INTO AirQuality (sensor_id, timestamp, pm25, pm10, co, no2, o3, so2) "
                 "VALUES (%d, '%s', %.2f, %.2f, %.2f, %.2f, %.2f, %.2f);",
            data.sensor_id, data.timestamp, data.pm25, data.pm10, data.co, data.no2, data.o3, data.so2);

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("Data inserted successfully!\n");
    }

    sqlite3_close(db);
}
