#include "aqm_db.h"
#include "globals.h"
#include <stdio.h>
#include <sqlite3.h>

static int ensure_sensor(sqlite3 *db, int sensor_id) {
    sqlite3_stmt *st = NULL;
    const char *sql = "INSERT OR IGNORE INTO sensors (id, name) VALUES (?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQLite prepare error: %s\n", sqlite3_errmsg(db));
        return -1;
    }
    char name[64];
    const char *forced_name = NULL;
    if (sensor_id == 22)
        forced_name = "DHT22";

    int n = forced_name ? snprintf(name, sizeof(name), "%s", forced_name)
                        : snprintf(name, sizeof(name), "Sensor %d", sensor_id);
    if (n < 0 || (size_t)n >= sizeof(name)) {
        sqlite3_finalize(st);
        return -1;
    }
    sqlite3_bind_int(st, 1, sensor_id);
    sqlite3_bind_text(st, 2, name, -1, SQLITE_TRANSIENT);
    sqlite3_step(st);
    sqlite3_finalize(st);
    return 0;
}

void insert_data(AirQualityData data) {
    sqlite3 *db = NULL;
    if (aqm_db_open(&db) != 0)
        return;

    if (ensure_sensor(db, data.sensor_id) != 0) {
        aqm_db_close(db);
        return;
    }

    sqlite3_stmt *st = NULL;
    const char *sql = "INSERT INTO readings (sensor_id, measured_at, pm25, pm10, co, no2, o3, so2) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQLite prepare error: %s\n", sqlite3_errmsg(db));
        aqm_db_close(db);
        return;
    }

    sqlite3_bind_int(st, 1, data.sensor_id);
    sqlite3_bind_text(st, 2, data.timestamp, -1, SQLITE_STATIC);
    sqlite3_bind_double(st, 3, (double)data.pm25);
    sqlite3_bind_double(st, 4, (double)data.pm10);
    sqlite3_bind_double(st, 5, (double)data.co);
    sqlite3_bind_double(st, 6, (double)data.no2);
    sqlite3_bind_double(st, 7, (double)data.o3);
    sqlite3_bind_double(st, 8, (double)data.so2);

    int rc = sqlite3_step(st);
    sqlite3_finalize(st);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(db));
    } else {
        printf("Data inserted successfully.\n");
    }

    aqm_db_close(db);
}
