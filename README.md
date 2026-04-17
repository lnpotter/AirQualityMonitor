# Air Quality Monitor

## Overview

Air Quality Monitor is a portable C application that collects, stores, and analyzes air quality–style readings (PM2.5, PM10, CO, NO2, O3, SO2). It uses **SQLite** for storage, optional **libharu** for PDF export, and runs on **Linux**, **macOS**, and **Windows** (e.g. **MSYS2** / **MinGW-w64**).

Legacy code (pre–refactor snapshot) is preserved on the **`legacy`** branch; active development targets **`main`**.

## Features

- **Structured database**: normalized `sensors` + `readings` tables, foreign keys, indexes, migration from the old single-table `SensorData` schema when present.
- **Portable paths**: database and config live under `./data/` by default (`air_quality.db`, `config.cfg`). Override with `AIR_QUALITY_DATA_DIR`.
- **Safe SQL**: parameterized inserts; SQLite backup API for database copies (no `cp` / `copy` shell commands).
- **Pollutant limits** with alerts evaluated against the **latest** stored sample.
- **Collection**: simulated readings at a configurable interval (count prompted at runtime).
- **Export**: CSV with correct column semantics.
- **Statistics**: per-sensor aggregates (avg / max / min) for each pollutant.
- **Optional PDF**: built when compiled with `HAVE_HPDF` and linked against libharu.

## Requirements

| Component   | Required | Notes |
|------------|----------|--------|
| C compiler | Yes      | GCC or Clang |
| SQLite 3   | Yes      | Development headers (`libsqlite3-dev`, `sqlite-devel`, MSYS `pacman -S mingw-w64-x86_64-sqlite`, etc.) |
| libharu    | Optional | For PDF menu item; omit with `make HAVE_HPDF=0` |

**Removed / optional vs old README**: `ncurses`, `wiringPi`, and dynamic loading via `dlopen` are no longer required for the default build (simpler Windows/macOS/Linux parity).

### Debian / Ubuntu (example)

```sh
sudo apt-get install build-essential pkg-config libsqlite3-dev libhpdf-dev
```

### macOS (Homebrew example)

```sh
brew install sqlite libharu
```

### Windows (MSYS2 MinGW64 example)

```sh
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-sqlite mingw-w64-x86_64-libhpdf make
```

Build without PDF if libhpdf is unavailable:

```sh
make HAVE_HPDF=0
```

## File layout (main sources)

```
AirQualityMonitor/
├── Makefile
├── README.md
├── main.c
├── globals.h / globals.c
├── aqm_paths.c / aqm_paths.h      # data directory and path helpers
├── aqm_platform.c / aqm_platform.h # sleep, mkdir, stdin flush, trims
├── aqm_db.c / aqm_db.h            # schema, migration, sqlite helpers
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
└── sensor.h
```

## Build

```sh
make
```

Produces `air_quality_monitor` (on Windows with MinGW, the file may appear as `air_quality_monitor.exe`).

```sh
make clean
```

## Configuration and data

| Item | Default |
|------|---------|
| Data directory | `./data/` (created automatically) |
| Database | `./data/air_quality.db` |
| Config | `./data/config.cfg` |
| Override | Set environment variable `AIR_QUALITY_DATA_DIR` to an absolute or relative directory path |

CSV export writes `sensor_data.csv` in the **current working directory**. PDF output is `sensor_data_report.pdf` in the CWD when PDF support is enabled.

## Usage

```sh
./air_quality_monitor
```

On first run, if no config exists, the program prompts for limits and interval settings, then saves them under `./data/config.cfg`.

### Menu

1. Start data collection (number of samples × interval from config)  
2. Fetch data (recent rows, plain text table)  
3. Check alerts (latest sample vs limits)  
4. Export to CSV  
5. Generate statistics  
6. Backup database (SQLite backup API → `data/backup_<timestamp>_air_quality.db`)  
7. Cleanup old data (retention in days)  
8. Generate PDF report (if built with libharu)  
9. Configure limits and settings  
0. Exit  

## Branches

- **`main`**: current refactored codebase.  
- **`legacy`**: snapshot of the previous layout (single-table assumptions, `cp` backup, `ncurses` fetch, etc.) preserved for comparison.

## License

See `LICENSE` in the repository.
