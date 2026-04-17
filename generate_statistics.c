#include "aqm_db.h"
#include <stdio.h>
#include <sqlite3.h>

void generate_statistics(void) {
    sqlite3 *db = NULL;
    if (aqm_db_open(&db) != 0)
        return;

    sqlite3_stmt *res = NULL;
    const char *sql =
        "SELECT r.sensor_id, s.name, "
        "AVG(r.pm25), MAX(r.pm25), MIN(r.pm25), "
        "AVG(r.pm10), MAX(r.pm10), MIN(r.pm10), "
        "AVG(r.co), MAX(r.co), MIN(r.co), "
        "AVG(r.no2), MAX(r.no2), MIN(r.no2), "
        "AVG(r.o3), MAX(r.o3), MIN(r.o3), "
        "AVG(r.so2), MAX(r.so2), MIN(r.so2), "
        "COUNT(*) "
        "FROM readings r JOIN sensors s ON s.id = r.sensor_id "
        "GROUP BY r.sensor_id, s.name "
        "ORDER BY r.sensor_id;";

    if (sqlite3_prepare_v2(db, sql, -1, &res, NULL) != SQLITE_OK) {
        fprintf(stderr, "Query failed: %s\n", sqlite3_errmsg(db));
        aqm_db_close(db);
        return;
    }

    printf("\n--- Aggregated statistics by sensor ---\n");

    while (sqlite3_step(res) == SQLITE_ROW) {
        int sid = sqlite3_column_int(res, 0);
        const char *name = (const char *)sqlite3_column_text(res, 1);
        double count = sqlite3_column_double(res, 20);

        printf("\nSensor %d (%s) — samples: %.0f\n", sid, name ? name : "", count);
        printf("  PM2.5  avg %.4f  max %.4f  min %.4f\n", sqlite3_column_double(res, 2),
               sqlite3_column_double(res, 3), sqlite3_column_double(res, 4));
        printf("  PM10   avg %.4f  max %.4f  min %.4f\n", sqlite3_column_double(res, 5),
               sqlite3_column_double(res, 6), sqlite3_column_double(res, 7));
        printf("  CO     avg %.4f  max %.4f  min %.4f\n", sqlite3_column_double(res, 8),
               sqlite3_column_double(res, 9), sqlite3_column_double(res, 10));
        printf("  NO2    avg %.6f  max %.6f  min %.6f\n", sqlite3_column_double(res, 11),
               sqlite3_column_double(res, 12), sqlite3_column_double(res, 13));
        printf("  O3     avg %.6f  max %.6f  min %.6f\n", sqlite3_column_double(res, 14),
               sqlite3_column_double(res, 15), sqlite3_column_double(res, 16));
        printf("  SO2    avg %.6f  max %.6f  min %.6f\n", sqlite3_column_double(res, 17),
               sqlite3_column_double(res, 18), sqlite3_column_double(res, 19));
    }

    sqlite3_finalize(res);
    aqm_db_close(db);
}
