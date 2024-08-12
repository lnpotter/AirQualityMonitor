
# Air Quality Monitor

## Overview

The Air Quality Monitor is a C-based application designed to collect, store, and analyze air quality data. The system allows users to configure pollutant limits, set data collection intervals, and generate various reports including PDF and CSV exports. This project supports the monitoring of several pollutants such as PM2.5, PM10, CO, NO2, O3, and SO2.

## Features

- **Database Management**: Automatically creates a SQLite database to store air quality data.
- **Pollutant Limit Configuration**: Users can configure pollutant limits which will trigger alerts if exceeded.
- **Data Collection**: The system collects data at user-defined intervals and stores it in the database.
- **Alerts**: Automatically checks for pollutant levels that exceed the configured limits and can send alerts.
- **Data Export**: Export collected data to CSV format.
- **Statistics Generation**: Generate average, max, and min statistics for the collected data.
- **PDF Reporting**: Generate PDF reports of the collected data.
- **Backup and Cleanup**: Automatic backup of the database and cleanup of old data.
- **Configuration Persistence**: Saves user settings in a configuration file for future use.

## Requirements

To compile and run the project, you will need:

- GCC Compiler
- SQLite3
- Ncurses Library
- libharu for PDF generation
- wiringPi Library for sensor interaction

On a Debian-based system, you can install the necessary packages with:

```sh
sudo apt-get install gcc sqlite3 libsqlite3-dev libncurses5-dev libncursesw5-dev libhpdf-dev wiringpi
```

## File Structure

```
AirQualityMonitor/
├── Makefile
├── main.c
├── globals.h
├── globals.c
├── create_database.c
├── insert_data.c
├── fetch_data.c
├── alert_system.c
├── export_to_csv.c
├── configure_limits.c
├── generate_statistics.c
├── backup_database.c
├── data_cleanup.c
├── interval_collection.c
├── generate_pdf_report.c
├── config_persistence.c
├── dht22.c
```

## Compilation

To compile the project, run:

```sh
make
```

This will generate a single executable file named `air_quality_monitor`.

## Execution

To execute the program, run:

```sh
./air_quality_monitor
```

On the first run, the program will ask you to configure the settings (pollutant limits, collection intervals, etc.). These settings will be saved to a configuration file for future use.

## Usage

After launching the program, you will be presented with a menu that allows you to:

1. Start data collection
2. Fetch data
3. Check alerts
4. Export to CSV
5. Generate statistics
6. Backup database
7. Cleanup old data
8. Generate PDF report
9. Configure limits and settings
10. Exit the program

The program is designed to be extendable, allowing for easy addition of new features as needed.
