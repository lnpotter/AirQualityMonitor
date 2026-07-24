# Air Quality Monitor

[![CI](https://github.com/lnpotter/AirQualityMonitor/actions/workflows/ci.yml/badge.svg)](https://github.com/lnpotter/AirQualityMonitor/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-see%20LICENSE-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C99-orange.svg)](https://en.wikipedia.org/wiki/C99)
[![Platforms](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](#requirements)

Air Quality Monitor is a portable C application that collects, stores, and analyzes air quality–style readings (PM2.5, PM10, CO, NO2, O3, SO2). It uses **SQLite** for storage, optional **libharu** for PDF export, and runs on **Linux**, **macOS**, and **Windows** (e.g. **MSYS2** / **MinGW-w64**).

The core design problem this project solves: sensor hardware isn't always available (in CI, on a dev machine, or during a demo), but the rest of the system -- database, alerts, exports, statistics -- still needs to be built and tested against realistic data. The solution is a **runtime-loadable plugin architecture**: sensors are `.so`/`.dylib`/`.dll` modules loaded via `dlopen`/`LoadLibrary`, validated against a versioned ABI, and each one falls back to simulated data automatically when real hardware isn't reachable. The same binary and menu work identically with or without a sensor plugged in.

Legacy code (pre-refactor snapshot) is preserved on the **`legacy`** branch; active development targets **`main`**.

## Demo

[![asciicast](https://asciinema.org/a/1261473.svg)](https://asciinema.org/a/1261473)

## Architecture

```
                    +--------------------+
                    |   main.c (menu)    |
                    +---------+----------+
                              |
        +---------------------+---------------------+
        |                     |                      |
 +------v------+     +--------v--------+     +-------v-------+
 |  aqm_db.c   |     | sensor_loader.c |     | config_        |
 |  (SQLite)   |     |  (plugin ABI)   |     | persistence.c  |
 +-------------+     +--------+--------+     +----------------+
                              |
                    dlopen / LoadLibrary
                              |
              +---------------+----------------+
              |                |                |
     +--------v-------+ +------v---------+ +----v-----------+
     | dht22_plugin.so | |pms5003_plugin.so| |  ...more .so   |
     | (real GPIO or   | | (real UART or   | |  plugins       |
     |  mock fallback) | |  mock fallback) | |                |
     +-----------------+ +-----------------+ +----------------+
```

Every plugin exports a single `SensorPlugin sensor_plugin` symbol matching the ABI defined in `sensor.h`. `sensor_module_load()` rejects any plugin whose `api_version` doesn't match `SENSOR_PLUGIN_API_VERSION`, or that's missing a required function pointer, before the plugin is ever called -- a stale or incompatible module fails loudly at load time instead of crashing later. See `docs/plugins.md` for the full plugin authoring guide.

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

## Testing and code quality

- **Unit tests** (Unity): 39 tests across 3 suites -- string/mode validation, PMS5003 frame parsing and checksum validation, and config-value parsing (`aqm_parse_int`/`aqm_parse_float`/`aqm_trim_crlf`). Run with `make test`.
- **Static analysis** (cppcheck): run in CI on every push -- a full report plus a stricter pass that fails the build on real warnings/errors.
- **API documentation** (Doxygen): generated from comments in `include/`. See [Documentation](#documentation) below.
- **Continuous integration**: builds the app and plugins, runs the test suite, and runs cppcheck on every push/PR to `main`. See the badge at the top of this file.

## Documentation

Full API reference generated with Doxygen: **https://lnpotter.github.io/AirQualityMonitor/**

To regenerate locally:

```sh
doxygen Doxyfile
```

Output lands in `docs/api/html/` (gitignored).

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

## Build

```sh
make
```

Produces `air_quality_monitor` (on Windows with MinGW, the file may appear as `air_quality_monitor.exe`).

```sh
make test    # build and run the Unity test suites
make clean
```

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

## Known limitations

- **No non-interactive/CLI mode.** All interaction goes through the numbered menu (`stdin`); there's no way to script a single action (e.g. "collect 10 samples and exit") without driving the menu. This is also why `config_persistence.c` and the menu-handling functions in `main.c` aren't unit tested -- they're coupled to live terminal I/O.
- **Test coverage is partial.** Unity tests cover the pure/isolated modules (string validation, frame parsing, config-value parsing). Statistics aggregation (`generate_statistics.c`, `export_to_csv.c`'s `update_sensor_stats`) and alert evaluation (`alert_system.c`) are not yet covered.
- **`insert_data()` is dead code.** Superseded by `insert_data_sqlite()` (which takes an already-open connection), but kept for now -- see the `@deprecated` note in `include/data/insert_data.h`.
- **Global mutable state.** Sensor configuration and pollutant limits are process-wide globals (`globals.c`), not passed through a context struct. Workable at this scale; would need to change if the collection logic were ever made concurrent.

## License

See [LICENSE](LICENSE) in the repository.