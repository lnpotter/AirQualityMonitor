#include <stdio.h>
#include <sqlite3.h>

#define DB_NAME "air_quality.db"
#define CSV_FILE "air_quality_data.csv"

void export_to_csv() {
    sqlite3 *db;
    sqlite3_stmt *res;

    int rc = sqlite3_open(DB_NAME, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    FILE *csv_file = fopen(CSV_FILE, "w");
    if (!csv_file) {
        fprintf(stderr, "Cannot open CSV file for writing.\n");
        return;
    }

    char *sql = "SELECT * FROM AirQuality;";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        fclose(csv_file);
        return;
    }

    fprintf(csv_file, "ID,Sensor ID,Timestamp,PM2.5,PM10,CO,NO2,O3,SO2\n");
    while (sqlite3_step(res) == SQLITE_ROW) {
        fprintf(csv_file, "%d,%d,%s,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
                sqlite3_column_int(res, 0),
                sqlite3_column_int(res, 1),
                sqlite3_column_text(res, 2),
                sqlite3_column_double(res, 3),
                sqlite3_column_double(res, 4),
                sqlite3_column_double(res, 5),
                sqlite3_column_double(res, 6),
                sqlite3_column_double(res, 7),
                sqlite3_column_double(res, 8));
    }

    printf("Data exported to %s successfully!\n", CSV_FILE);

    sqlite3_finalize(res);
    sqlite3_close(db);
    fclose(csv_file);
}
