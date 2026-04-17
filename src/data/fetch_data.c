#include "core/aqm_db.h"
#include <stdio.h>
#include <sqlite3.h>

void fetch_data(void) {
    sqlite3 *db = NULL;
    if (aqm_db_open(&db) != 0)
        return;

    sqlite3_stmt *res = NULL;
    const char *sql =
        "SELECT r.id, r.sensor_id, s.name, r.model, r.measured_at, r.pm25, r.pm10, r.co, r.no2, r.o3, r.so2 "
        "FROM readings r JOIN sensors s ON s.id = r.sensor_id "
        "ORDER BY r.measured_at DESC LIMIT 200;";

    if (sqlite3_prepare_v2(db, sql, -1, &res, NULL) != SQLITE_OK) {
        fprintf(stderr, "Query failed: %s\n", sqlite3_errmsg(db));
        aqm_db_close(db);
        return;
    }

    printf("\n--- Recent readings (newest first, max 200) ---\n");
    printf("%-6s %-4s %-16s %-12s %-19s %8s %8s %8s %8s %8s %8s\n", "id", "sid", "sensor", "model", "measured_at", "pm25",
           "pm10", "co", "no2", "o3", "so2");

    while (sqlite3_step(res) == SQLITE_ROW) {
        int id = sqlite3_column_int(res, 0);
        int sid = sqlite3_column_int(res, 1);
        const char *sname = (const char *)sqlite3_column_text(res, 2);
        const char *model = (const char *)sqlite3_column_text(res, 3);
        const char *ts = (const char *)sqlite3_column_text(res, 4);
        double pm25 = sqlite3_column_double(res, 5);
        double pm10 = sqlite3_column_double(res, 6);
        double co = sqlite3_column_double(res, 7);
        double no2 = sqlite3_column_double(res, 8);
        double o3 = sqlite3_column_double(res, 9);
        double so2 = sqlite3_column_double(res, 10);

        printf("%-6d %-4d %-16s %-12s %-19s %8.2f %8.2f %8.2f %8.4f %8.4f %8.4f\n", id, sid, sname ? sname : "",
               model ? model : "", ts ? ts : "", pm25, pm10, co, no2, o3, so2);
    }

    sqlite3_finalize(res);
    aqm_db_close(db);
}
