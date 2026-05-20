#include "core/aqm_db.h"
#include "core/aqm_paths.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int exec_sql(sqlite3 *db, const char *sql) {
    char *err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQLite error: %s\n", err ? err : sqlite3_errmsg(db));
        sqlite3_free(err);
        return -1;
    }
    return 0;
}

static int table_exists(sqlite3 *db, const char *name) {
    sqlite3_stmt *st = NULL;
    const char *sql = "SELECT 1 FROM sqlite_master WHERE type='table' AND name=?;";
    if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK)
        return 0;
    sqlite3_bind_text(st, 1, name, -1, SQLITE_STATIC);
    int exists = sqlite3_step(st) == SQLITE_ROW;
    sqlite3_finalize(st);
    return exists;
}

static int column_exists(sqlite3 *db, const char *table, const char *column) {
    sqlite3_stmt *st = NULL;
    char sql[128];
    int n = snprintf(sql, sizeof(sql), "PRAGMA table_info(%s);", table);
    if (n < 0 || (size_t)n >= sizeof(sql))
        return 0;
    if (sqlite3_prepare_v2(db, sql, -1, &st, NULL) != SQLITE_OK)
        return 0;
    int found = 0;
    while (sqlite3_step(st) == SQLITE_ROW) {
        const char *name = (const char *)sqlite3_column_text(st, 1);
        if (name && strcmp(name, column) == 0) {
            found = 1;
            break;
        }
    }
    sqlite3_finalize(st);
    return found;
}

static int migrate_legacy_sensor_data(sqlite3 *db) {
    if (!table_exists(db, "SensorData"))
        return 0;

    if (exec_sql(db, "PRAGMA foreign_keys = OFF;") != 0)
        return -1;

    if (exec_sql(db,
                 "INSERT OR IGNORE INTO sensors (id, name) "
                 "SELECT DISTINCT sensor_id, 'Sensor ' || CAST(sensor_id AS TEXT) FROM SensorData;") != 0) {
        exec_sql(db, "PRAGMA foreign_keys = ON;");
        return -1;
    }

    if (exec_sql(db,
                 "INSERT INTO readings (sensor_id, measured_at, model, pm25, pm10, co, no2, o3, so2) "
                 "SELECT sensor_id, timestamp, 'legacy', pm25, pm10, co, no2, o3, so2 FROM SensorData;") != 0) {
        exec_sql(db, "PRAGMA foreign_keys = ON;");
        return -1;
    }

    if (exec_sql(db, "DROP TABLE IF EXISTS SensorData;") != 0) {
        exec_sql(db, "PRAGMA foreign_keys = ON;");
        return -1;
    }

    if (exec_sql(db, "PRAGMA foreign_keys = ON;") != 0)
        return -1;

    printf("Migrated legacy table SensorData into normalized schema.\n");
    return 0;
}

int aqm_db_get_path(char *buf, size_t buflen) { return aqm_get_db_path(buf, buflen); }

int aqm_db_open(sqlite3 **out_db) {
    if (!out_db)
        return -1;
    char path[AQM_PATH_MAX];
    if (aqm_get_db_path(path, sizeof(path)) != 0) {
        fprintf(stderr, "Could not resolve database path.\n");
        return -1;
    }
    sqlite3 *db = NULL;
    if (sqlite3_open(path, &db) != SQLITE_OK || !db) {
        const char *errmsg = db ? sqlite3_errmsg(db) : "Failed to allocate database handle";
        fprintf(stderr, "Cannot open database: %s\n", errmsg);
        if (db)
            sqlite3_close(db);
        return -1;
    }
    sqlite3_busy_timeout(db, 5000);
    exec_sql(db, "PRAGMA foreign_keys = ON;");
    *out_db = db;
    return 0;
}

void aqm_db_close(sqlite3 *db) {
    if (db)
        sqlite3_close(db);
}

int aqm_db_init(void) {
    sqlite3 *db = NULL;
    if (aqm_db_open(&db) != 0)
        return -1;

    const char *parts[] = {
        "CREATE TABLE IF NOT EXISTS sensors ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name TEXT NOT NULL"
        ");",
        "CREATE TABLE IF NOT EXISTS readings ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  sensor_id INTEGER NOT NULL,"
        "  measured_at TEXT NOT NULL,"
        "  model TEXT NOT NULL DEFAULT '',"
        "  pm25 REAL NOT NULL,"
        "  pm10 REAL NOT NULL,"
        "  co REAL NOT NULL,"
        "  no2 REAL NOT NULL,"
        "  o3 REAL NOT NULL,"
        "  so2 REAL NOT NULL,"
        "  FOREIGN KEY (sensor_id) REFERENCES sensors(id),"
        "  UNIQUE (sensor_id, measured_at)"
        ");",
        "CREATE INDEX IF NOT EXISTS idx_readings_time ON readings(measured_at);",
        "CREATE INDEX IF NOT EXISTS idx_readings_sensor ON readings(sensor_id);",
    };

    for (size_t i = 0; i < sizeof(parts) / sizeof(parts[0]); i++) {
        if (exec_sql(db, parts[i]) != 0) {
            aqm_db_close(db);
            return -1;
        }
    }

    if (!column_exists(db, "readings", "model")) {
        if (exec_sql(db, "ALTER TABLE readings ADD COLUMN model TEXT NOT NULL DEFAULT '';") != 0) {
            aqm_db_close(db);
            return -1;
        }
    }
    if (exec_sql(db,
                 "UPDATE readings "
                 "SET model = CASE WHEN model IS NULL OR model = '' THEN "
                 "COALESCE((SELECT s.name FROM sensors s WHERE s.id = readings.sensor_id), 'unknown') "
                 "ELSE model END;") != 0) {
        aqm_db_close(db);
        return -1;
    }

    if (migrate_legacy_sensor_data(db) != 0) {
        aqm_db_close(db);
        return -1;
    }

    sqlite3_stmt *ins = NULL;
    const char *ensure = "INSERT OR IGNORE INTO sensors (id, name) VALUES (1, 'Default sensor');";
    if (sqlite3_prepare_v2(db, ensure, -1, &ins, NULL) != SQLITE_OK) {
        fprintf(stderr, "Prepare error: %s\n", sqlite3_errmsg(db));
        aqm_db_close(db);
        return -1;
    }
    sqlite3_step(ins);
    sqlite3_finalize(ins);

    char path[AQM_PATH_MAX];
    if (aqm_get_db_path(path, sizeof(path)) == 0)
        printf("Database ready at %s\n", path);

    aqm_db_close(db);
    return 0;
}
