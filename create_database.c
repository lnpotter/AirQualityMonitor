#include <stdio.h>
#include <sqlite3.h>

#define DB_NAME "air_quality.db"

void create_database() {
    sqlite3 *db;
    char *err_msg = 0;

    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char *sql = "CREATE TABLE IF NOT EXISTS SensorData ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "sensor_name TEXT,"
                "data REAL,"
                "timestamp DATETIME"
                ");";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("Database and table created successfully!\n");
    }

    sqlite3_close(db);
}