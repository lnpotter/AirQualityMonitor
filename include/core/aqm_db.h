#ifndef AQM_DB_H
#define AQM_DB_H

#include <stddef.h>
#include <sqlite3.h>

/**
 * Open DB, enable foreign keys, create schema, migrate legacy SensorData if present.
 */
int aqm_db_init(void);

/** Path used for SQLite (after aqm_paths_init). Exposed for modules that open their own handle. */
int aqm_db_get_path(char *buf, size_t buflen);

/** Open existing database (PRAGMA foreign_keys, busy timeout). */
int aqm_db_open(sqlite3 **out_db);

void aqm_db_close(sqlite3 *db);

#endif
