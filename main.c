#include "globals.h"
#include "sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

// Function declarations for each module
void create_database();
void insert_sensor_data(const char *sensor_name, float data);
void fetch_data();
void check_alerts();
void export_to_csv();
void configure_limits();
void generate_statistics();
void backup_database();
void cleanup_old_data();
void interval_collection();
void generate_pdf_report();
int load_config();
void save_config();
void show_menu();

void load_sensor_module(const char *module_path);

int main() {
    if (!load_config()) {
        printf("Configuration file not found. Please configure the settings.\n");
        configure_limits();
        save_config();
    } else {
        printf("Configuration loaded successfully.\n");
    }

    create_database();

    // Example: Load sensor modules
    load_sensor_module("./dht22.so");
    load_sensor_module("./mq135.so");

    show_menu();

    return 0;
}

void load_sensor_module(const char *module_path) {
    void *handle = dlopen(module_path, RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Failed to load sensor module: %s\n", dlerror());
        return;
    }

    Sensor *sensor = dlsym(handle, "sensor");
    if (!sensor) {
        fprintf(stderr, "Failed to load sensor: %s\n", dlerror());
        return;
    }

    sensor->init();
    float data = sensor->read_data();
    if (data != -1) {
        insert_sensor_data(sensor->name, data);
    }
    sensor->shutdown();

    dlclose(handle);
}

void insert_sensor_data(const char *sensor_name, float data) {
    sqlite3 *db;
    char *err_msg = 0;
    char sql[256];

    int rc = sqlite3_open("air_quality.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    snprintf(sql, sizeof(sql),
             "INSERT INTO SensorData (sensor_name, data, timestamp) VALUES ('%s', %.2f, datetime('now'));",
             sensor_name, data);

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("Data from %s sensor inserted successfully!\n", sensor_name);
    }

    sqlite3_close(db);
}
