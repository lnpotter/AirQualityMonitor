#include "core/aqm_db.h"
#include "core/aqm_paths.h"
#include "core/aqm_platform.h"
#include "core/globals.h"
#include "sensor/sensor_info.h"
#include "sensor/sensor_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void fetch_data(void);
void check_alerts(void);
void export_to_csv(void);
void configure_limits(void);
void generate_statistics(void);
void backup_database(void);
void cleanup_old_data(void);
void interval_collection(void);
void generate_pdf_report(void);
void insert_data(AirQualityData data);
int load_config(void);
void save_config(void);
void show_menu(void);
void configure_sensor_runtime(void);
void configure_multi_sensor(void);
void wait_for_menu_return(void);

int main(void) {
    aqm_paths_init();
    init_sensor_configs();
    srand((unsigned)time(NULL));

    if (!load_config()) {
        printf("Configuration file not found. Please configure the settings.\n");
        configure_limits();
        save_config();
    } else {
        printf("Configuration loaded successfully.\n");
    }

    if (aqm_db_init() != 0) {
        fprintf(stderr, "Database initialization failed.\n");
        return 1;
    }

    show_menu();
    return 0;
}

void show_menu(void) {
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
        printf("10. Show sensor runtime info\n");
        printf("11. Configure sensor/plugin runtime\n");
        printf("12. Auto-detect sensors\n");
        printf("13. Configure multi-sensor setup\n");
        printf("0. Exit\n");
        printf("Select an option: ");

        if (scanf("%d", &option) != 1) {
            aqm_flush_stdin();
            printf("Invalid input.\n");
            continue;
        }
        aqm_flush_stdin();

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
            case 10:
                show_sensor_runtime_info();
                break;
            case 11:
                configure_sensor_runtime();
                save_config();
                break;
            case 12:
                run_auto_detection_and_prompt();
                save_config();
                break;
            case 13:
                configure_multi_sensor();
                save_config();
                break;
            case 0:
                printf("Exiting program...\n");
                break;
            default:
                printf("Invalid option. Try again.\n");
        }

        if (option != 0)
            wait_for_menu_return();
    } while (option != 0);
}

void wait_for_menu_return(void) {
    char input[8];
    printf("\nPress Enter to return to the menu...");
    (void)fgets(input, sizeof(input), stdin);
}

void configure_sensor_runtime(void) {
    const char *allowed_modes[] = {"mock", "dht22", "bme680", "pms5003", "mh-z19", "mhz19"};
    const size_t allowed_count = sizeof(allowed_modes) / sizeof(allowed_modes[0]);
    char input[256];

    printf("\nSensor/plugin runtime configuration:\n");
    printf("Current mode: %s\n", sensor_mode);
    printf("Plugins enabled: %s\n", sensor_plugins_enabled ? "yes" : "no");
    printf("Custom plugin path: %s\n", sensor_plugin_path[0] ? sensor_plugin_path : "(none)");

    printf("Enable plugins? (1=yes, 0=no, Enter=keep): ");
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n')
        sensor_plugins_enabled = atoi(input) ? 1 : 0;

    printf("Sensor mode [mock|dht22|bme680|pms5003|mh-z19] (Enter=keep): ");
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        aqm_trim_crlf(input);
        if (input[0]) {
            int ok = 0;
            for (size_t i = 0; i < allowed_count; i++) {
                if (strcmp(input, allowed_modes[i]) == 0) {
                    ok = 1;
                    break;
                }
            }
            if (ok) {
                size_t n = strlen(input);
                if (n >= sizeof(sensor_mode))
                    n = sizeof(sensor_mode) - 1;
                memcpy(sensor_mode, input, n);
                sensor_mode[n] = '\0';
            } else {
                printf("Invalid sensor mode. Keeping current mode.\n");
            }
        }
    }

    printf("Custom plugin path (Enter=keep, '-'=clear): ");
    if (fgets(input, sizeof(input), stdin) && input[0] != '\n') {
        aqm_trim_crlf(input);
        if (strcmp(input, "-") == 0) {
            sensor_plugin_path[0] = '\0';
        } else if (input[0]) {
            snprintf(sensor_plugin_path, sizeof(sensor_plugin_path), "%s", input);
        }
    }

    printf("Runtime sensor settings updated.\n");
}

void configure_multi_sensor(void) {
    const char *allowed_modes[] = {"dht22", "bme680", "pms5003", "mhz19"};
    const size_t allowed_count = sizeof(allowed_modes) / sizeof(allowed_modes[0]);
    char input[256];

    printf("\n=== Multi-Sensor Configuration ===\n");
    printf("Currently configured sensors: %d\n", active_sensor_count);
    
    for (int i = 0; i < active_sensor_count; i++) {
        printf("  %d. Mode: %s, Path: %s, Enabled: %s\n", 
               i + 1, 
               sensor_configs[i].mode, 
               sensor_configs[i].plugin_path[0] ? sensor_configs[i].plugin_path : "(default)",
               sensor_configs[i].enabled ? "yes" : "no");
    }
    
    printf("\nOptions:\n");
    printf("1. Add a new sensor\n");
    printf("2. Remove a sensor\n");
    printf("3. Enable/Disable a sensor\n");
    printf("4. Clear all sensors\n");
    printf("0. Back to main menu\n");
    printf("Select an option: ");
    
    int option;
    if (scanf("%d", &option) != 1) {
        aqm_flush_stdin();
        printf("Invalid input.\n");
        return;
    }
    aqm_flush_stdin();
    
    switch (option) {
        case 1:
            if (active_sensor_count >= MAX_SENSORS) {
                printf("Maximum number of sensors (%d) reached.\n", MAX_SENSORS);
                return;
            }
            
            printf("\nAvailable sensor modes:\n");
            for (size_t i = 0; i < allowed_count; i++) {
                printf("  - %s\n", allowed_modes[i]);
            }
            
            printf("\nEnter sensor mode: ");
            if (fgets(input, sizeof(input), stdin)) {
                aqm_trim_crlf(input);
                int ok = 0;
                for (size_t i = 0; i < allowed_count; i++) {
                    if (strcmp(input, allowed_modes[i]) == 0) {
                        ok = 1;
                        break;
                    }
                }
                
                if (ok) {
                    SensorConfig *config = &sensor_configs[active_sensor_count];
                    snprintf(config->mode, sizeof(config->mode), "%s", input);
                    config->plugin_path[0] = '\0'; // Use default path
                    config->enabled = 1;
                    active_sensor_count++;
                    printf("Sensor %s added successfully.\n", input);
                } else {
                    printf("Invalid sensor mode.\n");
                }
            }
            break;
            
        case 2:
            if (active_sensor_count == 0) {
                printf("No sensors configured.\n");
                return;
            }
            
            printf("\nEnter sensor number to remove (1-%d): ", active_sensor_count);
            int idx;
            if (scanf("%d", &idx) == 1 && idx >= 1 && idx <= active_sensor_count) {
                aqm_flush_stdin();
                // Shift remaining sensors
                for (int i = idx - 1; i < active_sensor_count - 1; i++) {
                    sensor_configs[i] = sensor_configs[i + 1];
                }
                // Clear the last slot
                sensor_configs[active_sensor_count - 1].mode[0] = '\0';
                sensor_configs[active_sensor_count - 1].plugin_path[0] = '\0';
                sensor_configs[active_sensor_count - 1].enabled = 0;
                active_sensor_count--;
                printf("Sensor removed successfully.\n");
            } else {
                aqm_flush_stdin();
                printf("Invalid sensor number.\n");
            }
            break;
            
        case 3:
            if (active_sensor_count == 0) {
                printf("No sensors configured.\n");
                return;
            }
            
            printf("\nEnter sensor number to toggle (1-%d): ", active_sensor_count);
            int toggle_idx;
            if (scanf("%d", &toggle_idx) == 1 && toggle_idx >= 1 && toggle_idx <= active_sensor_count) {
                aqm_flush_stdin();
                sensor_configs[toggle_idx - 1].enabled = !sensor_configs[toggle_idx - 1].enabled;
                printf("Sensor %s is now %s.\n", 
                       sensor_configs[toggle_idx - 1].mode,
                       sensor_configs[toggle_idx - 1].enabled ? "enabled" : "disabled");
            } else {
                aqm_flush_stdin();
                printf("Invalid sensor number.\n");
            }
            break;
            
        case 4:
            printf("\nAre you sure you want to clear all sensors? (y/n): ");
            if (fgets(input, sizeof(input), stdin) && (input[0] == 'y' || input[0] == 'Y')) {
                init_sensor_configs();
                printf("All sensors cleared.\n");
            } else {
                printf("Operation canceled.\n");
            }
            break;
            
        case 0:
            return;
            
        default:
            printf("Invalid option.\n");
    }
}
