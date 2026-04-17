#include "core/aqm_db.h"
#include "core/aqm_paths.h"
#include "core/aqm_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void backup_database(void) {
    sqlite3 *src = NULL;
    sqlite3 *dest = NULL;

    if (aqm_db_open(&src) != 0)
        return;

    time_t t = time(NULL);
    struct tm *ptm = localtime(&t);
    if (!ptm) {
        fprintf(stderr, "localtime failed.\n");
        aqm_db_close(src);
        return;
    }
    struct tm tm = *ptm;

    char stamp[64];
    snprintf(stamp, sizeof(stamp), "%04d-%02d-%02d_%02d-%02d-%02d", tm.tm_year + 1900, tm.tm_mon + 1,
             tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    char dest_path[AQM_PATH_MAX];
    char base[AQM_PATH_MAX];
    if (aqm_get_data_dir(base, sizeof(base)) != 0) {
        aqm_db_close(src);
        fprintf(stderr, "Could not resolve data directory.\n");
        return;
    }

    {
        const char *prefix = "backup_";
        const char *suffix = "_air_quality.db";
        size_t need = strlen(base) + strlen(AQM_PATH_SEP) + strlen(prefix) + strlen(stamp) + strlen(suffix) + 1;
        if (need > sizeof(dest_path)) {
            fprintf(stderr, "Backup path is too long.\n");
            aqm_db_close(src);
            return;
        }
        strcpy(dest_path, base);
        strcat(dest_path, AQM_PATH_SEP);
        strcat(dest_path, prefix);
        strcat(dest_path, stamp);
        strcat(dest_path, suffix);
    }

    if (sqlite3_open(dest_path, &dest) != SQLITE_OK) {
        fprintf(stderr, "Cannot create backup file: %s\n", sqlite3_errmsg(dest));
        sqlite3_close(dest);
        aqm_db_close(src);
        return;
    }

    sqlite3_backup *b = sqlite3_backup_init(dest, "main", src, "main");
    if (!b) {
        fprintf(stderr, "Backup init failed: %s\n", sqlite3_errmsg(dest));
        sqlite3_close(dest);
        aqm_db_close(src);
        return;
    }

    (void)sqlite3_backup_step(b, -1);
    int rc = sqlite3_backup_finish(b);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Backup failed: %s\n", sqlite3_errmsg(dest));
    } else {
        printf("Backup created: %s\n", dest_path);
    }

    sqlite3_close(dest);
    aqm_db_close(src);
}
