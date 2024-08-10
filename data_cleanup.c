#include "globals.h"
#include <stdio.h>
#include <sqlite3.h>
#include <time.h>

#define DB_NAME "air_quality.db"

void cleanup_old_data() {
    sqlite3 *db;
    char *err_msg = 0;

    int rc = sqlite3_open(DB_NAME, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    char sql[256];
    snprintf(sql, sizeof(sql),
             "DELETE FROM AirQuality WHERE timestamp < datetime('now', '-%d days');", 
             retention_period);

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("Old data cleanup successful!\n");
    }

    sqlite3_close(db);
}
