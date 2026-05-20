#ifndef DATA_INSERT_DATA_H
#define DATA_INSERT_DATA_H

#include <sqlite3.h>
#include "core/globals.h"

int insert_data_sqlite(sqlite3 *db, const AirQualityData *data);
void insert_data(AirQualityData data);

#endif
