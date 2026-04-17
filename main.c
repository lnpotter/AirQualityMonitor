#include "aqm_db.h"
#include "aqm_paths.h"
#include "aqm_platform.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
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

int main(void) {
    aqm_paths_init();
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
            case 0:
                printf("Exiting program...\n");
                break;
            default:
                printf("Invalid option. Try again.\n");
        }

    } while (option != 0);
}
