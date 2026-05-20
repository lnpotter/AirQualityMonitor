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
│   ├── core/          # db, paths, platform, globals
│   └── sensor/        # plugin ABI/loader/runtime info headers
├── src/
│   ├── app/
│   │   └── main.c
│   ├── core/          # db, paths, platform, global defaults
│   ├── config/        # limits + config persistence
│   ├── data/          # insert/fetch/export/statistics
│   ├── sensor/        # collection flow + plugin loader/runtime info
│   ├── report/        # alerts + pdf report
│   └── maintenance/   # backup + cleanup
└── plugins/
    ├── dht22_plugin.c
    ├── bme680_plugin.c
    ├── pms5003_plugin.c
    └── mhz19_plugin.c
├── docs/
│   └── plugins.md
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

The collector supports portable **mock** mode and plugin-backed sensors.

- Sensor runtime is configured in menu option **11. Configure sensor/plugin runtime** and persisted in `config.cfg`.
- `sensor_mode`: `mock|dht22|bme680|pms5003|mh-z19|mhz19`
- `sensor_plugins_enabled`: `1|0` (enable/disable plugins globally)
- `sensor_plugin_path`: optional explicit module path override

Each reading now stores `model` in the database (for example `DHT22`, `MH-Z19`, `mock`).

### Multi-sensor setup

The application supports simultaneous data collection from multiple sensors (up to 8 sensors).

- Configure multi-sensor setup via menu option **13. Configure multi-sensor setup**
- Each sensor can be independently enabled/disabled
- Sensors are configured with mode (dht22, bme680, pms5003, mhz19) and optional custom plugin path
- Configuration is persisted in `config.cfg` with `sensor_N_mode`, `sensor_N_path`, `sensor_N_enabled` entries
- When multi-sensor mode is active, data collection reads from all enabled sensors for each sample

### Auto-detection

Menu option **12. Auto-detect sensors** automatically scans for available sensor plugins:

- Attempts to load each plugin from the `plugins/` directory
- Tests hardware initialization to determine if sensors are physically connected
- Offers to automatically enable detected sensors for multi-sensor data collection
- Useful for quick setup when hardware is available

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

Detailed authoring guide: `docs/plugins.md`.

### Example plugins included

- `plugins/bme680_plugin.c`
- `plugins/pms5003_plugin.c`
- `plugins/mhz19_plugin.c`

These examples run in simulated mode (no hardware required), useful for CI/testing/portfolio demos.
Hardware support in current plugins:

- `dht22_plugin`: real GPIO read on Linux/Raspberry Pi with `HAVE_WIRINGPI=1`; fallback mock otherwise.
- `mhz19_plugin`: real UART read on Linux/macOS (`MHZ19_DEVICE`, default `/dev/ttyS0`); fallback mock when device/read fails.
- `pms5003_plugin`: real UART frame read on Linux/macOS (`PMS5003_DEVICE`, default `/dev/ttyUSB0`); fallback mock when device/read fails.
- `bme680_plugin`: real read from Linux IIO/sysfs (`BME680_IIO_PATH` optional, otherwise auto-scan `/sys/bus/iio/devices`); fallback mock on unsupported systems or missing driver.

Build plugin shared libraries:

```sh
make plugins
```

On Linux this produces `.so`, on macOS `.dylib`, on Windows `.dll`.

For DHT22 hardware mode:

```sh
make plugins HAVE_WIRINGPI=1
```

Run with a plugin:

Plugins are configured via the menu system (option 11) and persisted in `config.cfg`. The application will automatically load the appropriate plugin based on the configured sensor mode. No environment variables are required for plugin loading.

### Real hardware connection quick notes

- **DHT22**: VCC + GND + DATA to GPIO, with 4.7k-10k pull-up on DATA. Optional `DHT22_PIN` (wiringPi pin number), default `7`.
- **MH-Z19**: UART TTL (`TX/RX/GND`) on serial adapter/UART pins. Set `MHZ19_DEVICE` (e.g. `/dev/ttyUSB0`).
- **PMS5003**: UART TTL (`TX/RX/GND` + power). Set `PMS5003_DEVICE` (e.g. `/dev/ttyUSB0`).
- **BME680**: I2C sensor with kernel driver exposing IIO files. Optionally set `BME680_IIO_PATH` directly (e.g. `/sys/bus/iio/devices/iio:device0`).

If real reading fails, plugin automatically falls back to mock data so the app stays operational.

## Usage

```sh
./air_quality_monitor
```

On first run, if no config exists, the program prompts for limits and settings, then saves them under `./data/config.cfg`.

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
11. Configure sensor/plugin runtime (enable/disable plugins, choose mode/path)  
12. Auto-detect sensors (scan for available sensor plugins and hardware)  
13. Configure multi-sensor setup (manage multiple sensors for simultaneous collection)  
0. Exit  

After each action, the program waits for Enter before returning to the menu.

## Branches

- **`main`**: current refactored codebase.  
- **`legacy`**: snapshot of the previous layout (single-table assumptions, `cp` backup, `ncurses` fetch, etc.) preserved for comparison.

## License

See `LICENSE` in the repository.
