#ifndef AQM_DB_H
#define AQM_DB_H

#include <stddef.h>
#include <sqlite3.h>

/**
 * @brief Open the database, enable foreign keys, create the schema, and
 *        migrate the legacy single-table `SensorData` schema if present.
 *
 * Intended to be called once at startup, before any other aqm_db_*
 * function.
 * @return 0 on success, non-zero on failure.
 */
int aqm_db_init(void);

/**
 * @brief Resolve the SQLite database file path (after aqm_paths_init()).
 *
 * Public accessor for external tooling/plugins that need to locate the
 * .db file without duplicating aqm_get_db_path()'s platform-specific
 * resolution logic. Not currently called internally.
 * @param buf Destination buffer for the NUL-terminated path.
 * @param buflen Size of buf in bytes.
 * @return 0 on success, non-zero on failure.
 */
int aqm_db_get_path(char *buf, size_t buflen);

/**
 * @brief Open the existing database with standard pragmas applied
 *        (foreign_keys on, busy timeout set).
 * @param out_db Receives the opened sqlite3 handle on success.
 * @return 0 on success, non-zero on failure.
 */
int aqm_db_open(sqlite3 **out_db);

/** @brief Close a database handle previously opened with aqm_db_open(). */
void aqm_db_close(sqlite3 *db);

#endif
