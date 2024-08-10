#include <stdio.h>
#include <sqlite3.h>
#include <ncurses.h>

#define DB_NAME "air_quality.db"

void fetch_data();
void display_graph(float value, float max_value);

void fetch_data() {
    sqlite3 *db;
    sqlite3_stmt *res;

    int rc = sqlite3_open(DB_NAME, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char *sql = "SELECT * FROM AirQuality;";

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    initscr();
    noecho();
    cbreak();

    printw("Air Quality Data:\n");
    while (sqlite3_step(res) == SQLITE_ROW) {
        float pm25 = sqlite3_column_double(res, 3);
        float pm10 = sqlite3_column_double(res, 4);
        float co = sqlite3_column_double(res, 5);
        float no2 = sqlite3_column_double(res, 6);
        float o3 = sqlite3_column_double(res, 7);
        float so2 = sqlite3_column_double(res, 8);

        printw("Sensor: %d | Timestamp: %s\n", sqlite3_column_int(res, 1), sqlite3_column_text(res, 2));
        printw("PM2.5: %.2f\n", pm25); display_graph(pm25, 50.0);
        printw("PM10: %.2f\n", pm10); display_graph(pm10, 100.0);
        printw("CO: %.2f\n", co); display_graph(co, 10.0);
        printw("NO2: %.2f\n", no2); display_graph(no2, 0.2);
        printw("O3: %.2f\n", o3); display_graph(o3, 0.2);
        printw("SO2: %.2f\n", so2); display_graph(so2, 0.1);
        printw("\n");
    }

    refresh();
    getch();
    endwin();

    sqlite3_finalize(res);
    sqlite3_close(db);
}

void display_graph(float value, float max_value) {
    int width = 50;
    int bar_length = (int)((value / max_value) * width);
    for (int i = 0; i < bar_length; i++) {
        printw("|");
    }
    printw("\n");
}
