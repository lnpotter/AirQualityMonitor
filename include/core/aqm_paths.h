#ifndef AQM_PATHS_H
#define AQM_PATHS_H

#include <stddef.h>

/** Maximum length (in bytes, including NUL) of any path buffer used by aqm_paths_*. */
#define AQM_PATH_MAX 512

/**
 * @brief Resolve and create (if needed) the data directory used for the
 *        database and config file.
 *
 * Priority: the `AIR_QUALITY_DATA_DIR` environment variable, falling
 * back to `./data` under the current working directory. Must be called
 * before aqm_get_data_dir()/aqm_get_db_path()/aqm_get_config_path().
 */
void aqm_paths_init(void);

/**
 * @brief Write the resolved data directory path into buf.
 * @param buf Destination buffer for the NUL-terminated path (no
 *        trailing separator).
 * @param buflen Size of buf in bytes.
 * @return 0 on success, non-zero on failure (e.g. buffer too small).
 */
int aqm_get_data_dir(char *buf, size_t buflen);

/**
 * @brief Write the full path to the SQLite database file into buf.
 * @param buf Destination buffer for the NUL-terminated path.
 * @param buflen Size of buf in bytes.
 * @return 0 on success, non-zero on failure.
 */
int aqm_get_db_path(char *buf, size_t buflen);

/**
 * @brief Write the full path to the config file into buf
 *        (`config.cfg`, alongside the database in the data directory).
 * @param buf Destination buffer for the NUL-terminated path.
 * @param buflen Size of buf in bytes.
 * @return 0 on success, non-zero on failure.
 */
int aqm_get_config_path(char *buf, size_t buflen);

#endif
