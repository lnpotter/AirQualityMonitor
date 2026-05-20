#include "core/aqm_db.h"
#include "core/globals.h"
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
    switch (sensor_id) {
        case 22:
            forced_name = "DHT22";
            break;
        case 680:
            forced_name = "BME680";
            break;
        case 5003:
            forced_name = "PMS5003";
            break;
        case 1900:
            forced_name = "MH-Z19";
            break;
        default:
            forced_name = NULL;
            break;
    }

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

static const char *default_model_name(int sensor_id) {
    switch (sensor_id) {
        case 22:
            return "DHT22";
        case 680:
            return "BME680";
        case 5003:
            return "PMS5003";
        case 1900:
            return "MH-Z19";
        default:
            return "unknown";
    }
}

static int insert_data_db(sqlite3 *db, const AirQualityData *data) {
    if (!db || !data)
        return -1;

    if (ensure_sensor(db, data->sensor_id) != 0)
        return -1;

    sqlite3_stmt *st = NULL;
    const char *sql = "INSERT INTO readings (sensor_id, measured_at, model, pm25, pm10, co, no2, o3, so2) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQLite prepare error: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_int(st, 1, data->sensor_id);
    sqlite3_bind_text(st, 2, data->timestamp, -1, SQLITE_STATIC);
    const char *model = data->model[0] ? data->model : default_model_name(data->sensor_id);
    sqlite3_bind_text(st, 3, model, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(st, 4, (double)data->pm25);
    sqlite3_bind_double(st, 5, (double)data->pm10);
    sqlite3_bind_double(st, 6, (double)data->co);
    sqlite3_bind_double(st, 7, (double)data->no2);
    sqlite3_bind_double(st, 8, (double)data->o3);
    sqlite3_bind_double(st, 9, (double)data->so2);

    int rc = sqlite3_step(st);
    sqlite3_finalize(st);

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    const char *model_name = data->model[0] ? data->model : default_model_name(data->sensor_id);
    printf("Data from %s (ID: %d) inserted successfully.\n", model_name, data->sensor_id);
    return 0;
}

int insert_data_sqlite(sqlite3 *db, const AirQualityData *data) {
    return insert_data_db(db, data);
}

void insert_data(AirQualityData data) {
    sqlite3 *db = NULL;
    if (aqm_db_open(&db) != 0)
        return;

    insert_data_db(db, &data);
    aqm_db_close(db);
}
