#include "core/aqm_db.h"
#include "core/globals.h"
#include <stdio.h>
#include <sqlite3.h>

static int exceeds_limits(const AirQualityData *d) {
    return (d->pm25 > limit_pm25 || d->pm10 > limit_pm10 || d->co > limit_co || d->no2 > limit_no2 ||
            d->o3 > limit_o3 || d->so2 > limit_so2);
}

void check_alerts(void) {
    sqlite3 *db = NULL;
    if (aqm_db_open(&db) != 0)
        return;

    sqlite3_stmt *st = NULL;
    const char *sql =
        "SELECT r.sensor_id, r.model, r.measured_at, r.pm25, r.pm10, r.co, r.no2, r.o3, r.so2 "
        "FROM readings r ORDER BY r.measured_at DESC LIMIT 1;";

    if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK) {
        fprintf(stderr, "Query failed: %s\n", sqlite3_errmsg(db));
        aqm_db_close(db);
        return;
    }

    if (sqlite3_step(st) != SQLITE_ROW) {
        printf("No readings in database yet.\n");
        sqlite3_finalize(st);
        aqm_db_close(db);
        return;
    }

    AirQualityData d;
    d.sensor_id = sqlite3_column_int(st, 0);
    const char *model = (const char *)sqlite3_column_text(st, 1);
    const char *ts = (const char *)sqlite3_column_text(st, 2);
    snprintf(d.model, sizeof(d.model), "%s", model ? model : "");
    snprintf(d.timestamp, sizeof(d.timestamp), "%s", ts ? ts : "");
    d.pm25 = (float)sqlite3_column_double(st, 3);
    d.pm10 = (float)sqlite3_column_double(st, 4);
    d.co = (float)sqlite3_column_double(st, 5);
    d.no2 = (float)sqlite3_column_double(st, 6);
    d.o3 = (float)sqlite3_column_double(st, 7);
    d.so2 = (float)sqlite3_column_double(st, 8);

    sqlite3_finalize(st);
    aqm_db_close(db);

    if (exceeds_limits(&d)) {
        printf("ALERT: Latest sample exceeds configured limits (sensor %d, model %s, %s).\n", d.sensor_id, d.model,
               d.timestamp);
        printf("  PM2.5 %.4f (limit %.4f)  PM10 %.4f (limit %.4f)  CO %.4f (limit %.4f)\n", d.pm25,
               limit_pm25, d.pm10, limit_pm10, d.co, limit_co);
        printf("  NO2 %.6f (limit %.6f)  O3 %.6f (limit %.6f)  SO2 %.6f (limit %.6f)\n", d.no2, limit_no2,
               d.o3, limit_o3, d.so2, limit_so2);
    } else {
        printf("Latest reading is within configured limits (sensor %d, model %s, %s).\n", d.sensor_id, d.model,
               d.timestamp);
    }
}
