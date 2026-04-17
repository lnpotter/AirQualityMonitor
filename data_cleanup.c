#include "aqm_db.h"
#include "globals.h"
#include <stdio.h>
#include <sqlite3.h>

void cleanup_old_data(void) {
    sqlite3 *db = NULL;
    if (aqm_db_open(&db) != 0)
        return;

    char sql[160];
    int n = snprintf(sql, sizeof(sql),
                     "DELETE FROM readings WHERE measured_at < datetime('now', '-%d days');",
                     retention_period);
    if (n < 0 || (size_t)n >= sizeof(sql)) {
        fprintf(stderr, "Invalid retention period.\n");
        aqm_db_close(db);
        return;
    }

    char *err = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
        fprintf(stderr, "Cleanup error: %s\n", err ? err : sqlite3_errmsg(db));
        sqlite3_free(err);
    } else {
        int changes = sqlite3_changes(db);
        printf("Cleanup complete. Rows removed: %d (retention: %d days).\n", changes, retention_period);
    }

    aqm_db_close(db);
}
