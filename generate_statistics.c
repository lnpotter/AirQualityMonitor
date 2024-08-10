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

    char *sql = "SELECT AVG(pm25), AVG(pm10), AVG(co), AVG(no2), AVG(o3), AVG(so2), "
                "MAX(pm25), MAX(pm10), MAX(co), MAX(no2), MAX(o3), MAX(so2), "
                "MIN(pm25), MIN(pm10), MIN(co), MIN(no2), MIN(o3), MIN(so2) "
                "FROM AirQuality;";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    if (sqlite3_step(res) == SQLITE_ROW) {
        printf("Statistics:\n");
        printf("Average PM2.5: %.2f, PM10: %.2f, CO: %.2f, NO2: %.2f, O3: %.2f, SO2: %.2f\n",
               sqlite3_column_double(res, 0),
               sqlite3_column_double(res, 1),
               sqlite3_column_double(res, 2),
               sqlite3_column_double(res, 3),
               sqlite3_column_double(res, 4),
               sqlite3_column_double(res, 5));

        printf("Max PM2.5: %.2f, PM10: %.2f, CO: %.2f, NO2: %.2f, O3: %.2f, SO2: %.2f\n",
               sqlite3_column_double(res, 6),
               sqlite3_column_double(res, 7),
               sqlite3_column_double(res, 8),
               sqlite3_column_double(res, 9),
               sqlite3_column_double(res, 10),
               sqlite3_column_double(res, 11));

        printf("Min PM2.5: %.2f, PM10: %.2f, CO: %.2f, NO2: %.2f, O3: %.2f, SO2: %.2f\n",
               sqlite3_column_double(res, 12),
               sqlite3_column_double(res, 13),
               sqlite3_column_double(res, 14),
               sqlite3_column_double(res, 15),
               sqlite3_column_double(res, 16),
               sqlite3_column_double(res, 17));
    }

    sqlite3_finalize(res);
    sqlite3_close(db);
}
