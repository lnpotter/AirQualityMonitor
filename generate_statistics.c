#include <stdio.h>
#include <sqlite3.h>

#define DB_NAME "air_quality.db"

void generate_statistics() {
    sqlite3 *db;
    sqlite3_stmt *res;

    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char *sql = "SELECT sensor_name, AVG(data), MAX(data), MIN(data) "
                "FROM SensorData GROUP BY sensor_name;";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    printf("Statistics:\n");
    while (sqlite3_step(res) == SQLITE_ROW) {
        const char *sensor_name = (const char *)sqlite3_column_text(res, 0);
        float avg = sqlite3_column_double(res, 1);
        float max = sqlite3_column_double(res, 2);
        float min = sqlite3_column_double(res, 3);

        printf("Sensor: %s | Average: %.2f | Max: %.2f | Min: %.2f\n", sensor_name, avg, max, min);
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
}
