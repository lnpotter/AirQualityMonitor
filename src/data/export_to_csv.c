#include "core/aqm_db.h"
#include <stdio.h>
#include <sqlite3.h>

#define CSV_FILE "sensor_data.csv"

void export_to_csv(void) {
    sqlite3 *db = NULL;
    if (aqm_db_open(&db) != 0)
        return;

    sqlite3_stmt *res = NULL;
    const char *sql =
        "SELECT r.id, r.sensor_id, s.name, r.model, r.measured_at, r.pm25, r.pm10, r.co, r.no2, r.o3, r.so2 "
        "FROM readings r JOIN sensors s ON s.id = r.sensor_id "
        "ORDER BY r.measured_at ASC;";

    if (sqlite3_prepare_v2(db, sql, -1, &res, NULL) != SQLITE_OK) {
        fprintf(stderr, "Query failed: %s\n", sqlite3_errmsg(db));
        aqm_db_close(db);
        return;
    }

    FILE *csv_file = fopen(CSV_FILE, "w");
    if (!csv_file) {
        fprintf(stderr, "Cannot open CSV file for writing: %s\n", CSV_FILE);
        sqlite3_finalize(res);
        aqm_db_close(db);
        return;
    }

    fprintf(csv_file, "id,sensor_id,sensor_name,model,measured_at,pm25,pm10,co,no2,o3,so2\n");

    while (sqlite3_step(res) == SQLITE_ROW) {
        const char *sname = (const char *)sqlite3_column_text(res, 2);
        const char *model = (const char *)sqlite3_column_text(res, 3);
        const char *mts = (const char *)sqlite3_column_text(res, 4);
        if (!sname)
            sname = "";
        if (!mts)
            mts = "";
        if (!model)
            model = "";
        fprintf(csv_file, "%d,%d,\"%s\",\"%s\",%s,%.4f,%.4f,%.4f,%.6f,%.6f,%.6f\n", sqlite3_column_int(res, 0),
                sqlite3_column_int(res, 1), sname, model, mts,
                sqlite3_column_double(res, 5), sqlite3_column_double(res, 6), sqlite3_column_double(res, 7),
                sqlite3_column_double(res, 8), sqlite3_column_double(res, 9), sqlite3_column_double(res, 10));
    }

    printf("Data exported to %s successfully.\n", CSV_FILE);

    sqlite3_finalize(res);
    aqm_db_close(db);
    fclose(csv_file);
}
