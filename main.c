#include "globals.h"
#include "sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <time.h>
#include <sqlite3.h>

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
void insert_data(AirQualityData data);
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
    //load_sensor_module("./dht22.so");

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
    AirQualityData aq;

    sensor->init();

    aq.sensor_id = 1;

    time_t now = time(NULL);
    strftime(aq.timestamp, sizeof(aq.timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

    float value = sensor->read_data();

    if (value != -1) {
        aq.pm25 = value;
        aq.pm10 = value;
        aq.co   = value;
        aq.no2  = value;
        aq.o3   = value;
        aq.so2  = value;

        insert_data(aq);
    }

    sensor->shutdown();

    dlclose(handle);
}

void show_menu() {
    int option;

    do {
        printf("\n=== Air Quality Monitor ===\n");
        printf("1. Start data collection\n");
        printf("2. Fetch data\n");
        printf("3. Check alerts\n");
        printf("4. Export to CSV\n");
        printf("5. Generate statistics\n");
        printf("6. Backup database\n");
        printf("7. Cleanup old data\n");
        printf("8. Generate PDF report\n");
        printf("9. Configure limits and settings\n");
        printf("0. Exit\n");
        printf("Select an option: ");

        if (scanf("%d", &option) != 1) {
            // Limpa entrada inválida
            while (getchar() != '\n');
            printf("Invalid input.\n");
            continue;
        }

        switch (option) {
            case 1:
                interval_collection();
                break;
            case 2:
                fetch_data();
                break;
            case 3:
                check_alerts();
                break;
            case 4:
                export_to_csv();
                break;
            case 5:
                generate_statistics();
                break;
            case 6:
                backup_database();
                break;
            case 7:
                cleanup_old_data();
                break;
            case 8:
                generate_pdf_report();
                break;
            case 9:
                configure_limits();
                save_config();
                break;
            case 0:
                printf("Exiting program...\n");
                break;
            default:
                printf("Invalid option. Try again.\n");
        }

    } while (option != 0);
}

