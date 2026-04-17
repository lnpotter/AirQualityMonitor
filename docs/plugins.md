# Sensor Plugins Guide

This project supports runtime-loadable sensor modules on Linux, macOS, and Windows.

## ABI Contract

Plugins must include `include/sensor.h` and export this symbol:

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
#include "../include/sensor.h"
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

Use explicit path:

```sh
AQM_SENSOR_PLUGIN=./plugins/bme680_plugin.so ./air_quality_monitor
```

Or auto-resolve by sensor name:

```sh
AQM_SENSOR=dht22 ./air_quality_monitor
AQM_SENSOR=bme680 ./air_quality_monitor
AQM_SENSOR=pms5003 ./air_quality_monitor
AQM_SENSOR=mh-z19 ./air_quality_monitor
```

On Windows PowerShell:

```powershell
$env:AQM_SENSOR_PLUGIN=".\plugins\bme680_plugin.dll"
.\air_quality_monitor.exe
```
