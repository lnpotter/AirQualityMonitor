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
- **Dynamic sensor modules**: runtime-loaded plugins (`dlopen` on Linux/macOS, `LoadLibrary` on Windows) via `AQM_SENSOR_PLUGIN`.
- **Export**: CSV with correct column semantics.
- **Statistics**: per-sensor aggregates (avg / max / min) for each pollutant.
- **Optional PDF**: built when compiled with `HAVE_HPDF` and linked against libharu.

## Requirements

| Component   | Required | Notes |
|------------|----------|--------|
| C compiler | Yes      | GCC or Clang |
| SQLite 3   | Yes      | Development headers (`libsqlite3-dev`, `sqlite-devel`, MSYS `pacman -S mingw-w64-x86_64-sqlite`, etc.) |
| libharu    | Optional | For PDF menu item; omit with `make HAVE_HPDF=0` |

**Optional dependencies**: the default build runs without GPIO libraries; `wiringPi` and dynamic sensor plugins are optional capabilities.

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
├── include/
│   ├── globals.h
│   ├── sensor.h
│   ├── sensor_loader.h
│   └── ...other headers
├── src/
│   ├── main.c
│   ├── aqm_db.c
│   ├── interval_collection.c
│   ├── sensor_loader.c
│   └── ...other modules
└── plugins/
    ├── dht22_plugin.c
    ├── bme680_plugin.c
    ├── pms5003_plugin.c
    └── mhz19_plugin.c
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

### Sensor selection (portable mocks)

The collector supports a portable **mock** mode and an optional **DHT22** mode selected by an environment variable:

- `AQM_SENSOR=mock` (default): generates realistic-ish pollutant values without requiring hardware.
- `AQM_SENSOR=dht22|bme680|pms5003|mh-z19`: auto-resolves plugin path by OS extension (`.so`, `.dylib`, `.dll`) and loads it.
- `AQM_SENSOR_PLUGIN=<path>`: loads a specific sensor module dynamically at runtime. When provided, this has priority over `AQM_SENSOR`.

**Current schema mapping for DHT22** (until we add dedicated columns): temperature (°C) is stored in `pm25`, and humidity (%) is stored in `pm10`.

### Dynamic plugin architecture

Runtime flow:

`Program -> load shared module -> resolve sensor_plugin symbol -> init/read/shutdown -> unload module`

Cross-platform loader:

- Linux/macOS: `dlopen` / `dlsym` / `dlclose`
- Windows: `LoadLibrary` / `GetProcAddress` / `FreeLibrary`

ABI contract is defined in `sensor.h` (`SensorPlugin`). A plugin must export:

```c
SensorPlugin sensor_plugin;
```

ABI safety:

- `api_version` is required and validated by the loader (`SENSOR_PLUGIN_API_VERSION`).
- mandatory metadata fields: `name`, `plugin_version`, `description`.

### Example plugins included

- `plugins/bme680_plugin.c`
- `plugins/pms5003_plugin.c`
- `plugins/mhz19_plugin.c`

These examples run in simulated mode (no hardware required), useful for CI/testing/portfolio demos.
`dht22_plugin` supports hardware mode on Linux/Raspberry Pi when compiled with `HAVE_WIRINGPI=1`; otherwise it falls back to simulated mode.

Build plugin shared libraries:

```sh
make plugins
```

On Linux this produces `.so`, on macOS `.dylib`, on Windows `.dll`.

Run with a plugin:

```sh
# Linux example
AQM_SENSOR_PLUGIN=./plugins/bme680_plugin.so ./air_quality_monitor
```

```sh
# macOS example
AQM_SENSOR_PLUGIN=./plugins/bme680_plugin.dylib ./air_quality_monitor
```

```powershell
# Windows PowerShell example
$env:AQM_SENSOR_PLUGIN=".\plugins\bme680_plugin.dll"
.\air_quality_monitor.exe
```

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
10. Show sensor runtime info (active mode/env/plugin metadata)  
0. Exit  

## Branches

- **`main`**: current refactored codebase.  
- **`legacy`**: snapshot of the previous layout (single-table assumptions, `cp` backup, `ncurses` fetch, etc.) preserved for comparison.

## License

See `LICENSE` in the repository.
