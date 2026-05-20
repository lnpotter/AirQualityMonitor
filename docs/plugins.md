# Sensor Plugins Guide

This project supports runtime-loadable sensor modules on Linux, macOS, and Windows.

## ABI Contract

Plugins must include `include/sensor/sensor.h` and export this symbol:

```c
SensorPlugin sensor_plugin;
```

Required fields in `SensorPlugin`:

- `api_version` (must match `SENSOR_PLUGIN_API_VERSION`)
- `name`
- `plugin_version`
- `description`
- `sensor_id`
- callbacks: `init`, `read_sample`, `shutdown`

If ABI validation fails, the loader rejects the plugin.

## Minimal Plugin Template

```c
#include "../include/sensor/sensor.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define SENSOR_PLUGIN_EXPORT __declspec(dllexport)
#else
#define SENSOR_PLUGIN_EXPORT
#endif

static int my_init(void) { return 0; }

static int my_read_sample(AirQualityData *out) {
    if (!out) return -1;
    memset(out, 0, sizeof(*out));
    out->sensor_id = 1234;

    time_t now = time(NULL);
    struct tm *ptm = localtime(&now);
    if (!ptm) return -1;
    snprintf(out->timestamp, sizeof(out->timestamp), "%04d-%02d-%02d %02d:%02d:%02d",
             ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday,
             ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

    // Map sensor metrics to current schema fields.
    out->pm25 = 10.0f;
    out->pm10 = 20.0f;
    out->co = 1.0f;
    out->no2 = 0.0f;
    out->o3 = 0.0f;
    out->so2 = 0.0f;
    return 0;
}

static int my_shutdown(void) { return 0; }

SENSOR_PLUGIN_EXPORT SensorPlugin sensor_plugin = {
    .api_version = SENSOR_PLUGIN_API_VERSION,
    .name = "MySensor",
    .plugin_version = "1.0.0",
    .description = "My custom sensor plugin.",
    .sensor_id = 1234,
    .init = my_init,
    .read_sample = my_read_sample,
    .shutdown = my_shutdown,
};
```

## Build Plugins

Build all official plugins:

```sh
make plugins
```

Output extension depends on OS:

- Linux: `.so`
- macOS: `.dylib`
- Windows: `.dll`

## Run with Plugins

Plugins are configured via the application's menu system (option 11: Configure sensor/plugin runtime) and persisted in `config.cfg`. The application will automatically load the appropriate plugin based on the configured sensor mode.

To use a plugin:
1. Build the plugin: `make plugins`
2. Run the application: `./air_quality_monitor`
3. Select option 11 from the menu
4. Enable plugins and select the desired sensor mode (dht22, bme680, pms5003, mhz19)
5. Optionally specify a custom plugin path if needed
6. Save the configuration

The application will automatically load the correct plugin file (.so on Linux, .dylib on macOS, .dll on Windows) based on the sensor mode and platform.
