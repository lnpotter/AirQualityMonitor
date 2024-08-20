#include <stdio.h>
#include <sqlite3.h>
#include <ncurses.h>

#define DB_NAME "air_quality.db"

void fetch_data() {
    sqlite3 *db;
    sqlite3_stmt *res;

    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char *sql = "SELECT * FROM SensorData;";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    initscr();
    noecho();
    cbreak();

    printw("Sensor Data:\n");
    while (sqlite3_step(res) == SQLITE_ROW) {
        const char *sensor_name = (const char *)sqlite3_column_text(res, 1);
        float data = sqlite3_column_double(res, 2);
        const char *timestamp = (const char *)sqlite3_column_text(res, 3);

        printw("Sensor: %s | Data: %.2f | Timestamp: %s\n", sensor_name, data, timestamp);
    }

    refresh();
    getch();
    endwin();

    sqlite3_finalize(res);
    sqlite3_close(db);
}
