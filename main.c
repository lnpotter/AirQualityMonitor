#include "globals.h"
#include <stdio.h>
#include <stdlib.h>

// Function declarations for each module
void create_database();
void insert_data();
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

int main() {
    // Attempt to load configurations from file
    if (!load_config()) {
        printf("Configuration file not found. Please configure the settings.\n");
        configure_limits();  // Prompt user to set limits and settings
        save_config();  // Save the initial configuration
    } else {
        printf("Configuration loaded successfully.\n");
    }

    create_database();  // Ensure database is ready

    show_menu();  // Display menu for user interaction

    return 0;
}

void show_menu() {
    int choice;
    while (1) {
        printf("\nAir Quality Monitor Menu:\n");
        printf("1. Start Data Collection\n");
        printf("2. Fetch Data\n");
        printf("3. Check Alerts\n");
        printf("4. Export to CSV\n");
        printf("5. Generate Statistics\n");
        printf("6. Backup Database\n");
        printf("7. Cleanup Old Data\n");
        printf("8. Generate PDF Report\n");
        printf("9. Configure Limits and Settings\n");
        printf("10. Exit\n");
        printf("Choose an option: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                interval_collection();
                break;
            case 2:
                fetch_data();
                break;
            case 3: {
                AirQualityData sample_data = {1, "", 1.5, 2.5, 3.5, 0.04, 0.03, 0.02}; // Example data for checking alerts
                check_alerts(sample_data);
                break;
            }
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
                save_config();  // Save updated settings
                break;
            case 10:
                printf("Exiting...\n");
                return;
            default:
                printf("Invalid choice, please try again.\n");
        }
    }
}
