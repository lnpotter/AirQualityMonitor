#ifndef AQM_PATHS_H
#define AQM_PATHS_H

#include <stddef.h>

#define AQM_PATH_MAX 512

/**
 * Resolve data directory (creates it on first use).
 * Priority: env AIR_QUALITY_DATA_DIR, then ./data under current working directory.
 */
void aqm_paths_init(void);

/** Writes NUL-terminated path to data directory (trailing separator omitted). */
int aqm_get_data_dir(char *buf, size_t buflen);

/** Full path to SQLite database file. */
int aqm_get_db_path(char *buf, size_t buflen);

/** Full path to config file (config.cfg next to DB in data dir). */
int aqm_get_config_path(char *buf, size_t buflen);

#endif
