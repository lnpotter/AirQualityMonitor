#ifndef DATA_INSERT_DATA_H
#define DATA_INSERT_DATA_H

#include <sqlite3.h>
#include "core/globals.h"

/**
 * @brief Insert one reading using an already-open database connection.
 *
 * Preferred over insert_data() below: takes the connection as a
 * parameter (dependency injection) instead of managing its own,
 * which is what makes it usable both from interval_collection.c's
 * collection loop and from unit tests with a temporary database.
 * @param db Open database handle (see aqm_db_open()).
 * @param data Reading to insert.
 * @return 0 on success, non-zero on failure.
 */
int insert_data_sqlite(sqlite3 *db, const AirQualityData *data);

#endif
