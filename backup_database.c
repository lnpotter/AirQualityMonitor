#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DB_NAME "air_quality.db"
#define BACKUP_FILE "air_quality_backup.db"

void backup_database() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char timestamp[64];
    snprintf(timestamp, sizeof(timestamp), "%d-%02d-%02d_%02d-%02d-%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);

    char backup_filename[256];
    snprintf(backup_filename, sizeof(backup_filename), "backup_%s_%s", timestamp, DB_NAME);

    char command[512];
    snprintf(command, sizeof(command), "cp %s %s", DB_NAME, backup_filename);

    if (system(command) == 0) {
        printf("Backup created successfully: %s\n", backup_filename);
    } else {
        printf("Failed to create backup.\n");
    }
}
