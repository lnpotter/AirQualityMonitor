#include "../include/sensor/sensor.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifndef _WIN32
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#ifndef CRTSCTS
#define CRTSCTS 0
#endif
#endif

#ifdef _WIN32
#define SENSOR_PLUGIN_EXPORT __declspec(dllexport)
#else
#define SENSOR_PLUGIN_EXPORT
#endif

static void fill_timestamp(char *buf, size_t len) {
    time_t now = time(NULL);
    struct tm *ptm = localtime(&now);
    if (!ptm) {
        snprintf(buf, len, "1970-01-01 00:00:00");
        return;
    }
    if (strftime(buf, len, "%Y-%m-%d %H:%M:%S", ptm) == 0) {
        snprintf(buf, len, "1970-01-01 00:00:00");
    }
}

static int mhz19_init(void) {
#ifndef _WIN32
    printf("MH-Z19 plugin initialized (real UART on Linux/macOS if available; fallback mock).\n");
#else
    printf("MH-Z19 plugin initialized (mock on Windows).\n");
#endif
    return 0;
}

#ifndef _WIN32
static int open_serial_9600(const char *device) {
    int fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0)
        return -1;
    struct termios tty;
    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(fd, &tty) != 0) {
        close(fd);
        return -1;
    }
    cfsetospeed(&tty, B9600);
    cfsetispeed(&tty, B9600);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~(IGNBRK | IXON | IXOFF | IXANY);
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 9;
    tty.c_cc[VTIME] = 2;
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD | CSTOPB | CRTSCTS);
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        close(fd);
        return -1;
    }
    tcflush(fd, TCIOFLUSH);
    return fd;
}

static unsigned char mhz19_checksum(const unsigned char *buf) {
    unsigned int sum = 0;
    for (int i = 1; i < 8; i++)
        sum += buf[i];
    return (unsigned char)(0xFF - (sum & 0xFF) + 1);
}

static int read_mhz19_ppm(float *ppm_out) {
    const char *dev = getenv("MHZ19_DEVICE");
    if (!dev || !dev[0])
        dev = "/dev/ttyS0";
    int fd = open_serial_9600(dev);
    if (fd < 0)
        return -1;

    unsigned char cmd[9] = {0xFF, 0x01, 0x86, 0, 0, 0, 0, 0, 0x79};
    if (write(fd, cmd, sizeof(cmd)) != (ssize_t)sizeof(cmd)) {
        close(fd);
        return -1;
    }

    unsigned char resp[9];
    ssize_t got = read(fd, resp, sizeof(resp));
    close(fd);
    if (got != (ssize_t)sizeof(resp))
        return -1;
    if (resp[0] != 0xFF || resp[1] != 0x86)
        return -1;
    if (resp[8] != mhz19_checksum(resp))
        return -1;
    int ppm = ((int)resp[2] << 8) | resp[3];
    if (ppm < 0 || ppm > 10000)
        return -1;
    *ppm_out = (float)ppm;
    return 0;
}
#endif

static int mhz19_read_sample(AirQualityData *out) {
    if (!out)
        return -1;
    memset(out, 0, sizeof(*out));
    out->sensor_id = 1900;
    snprintf(out->model, sizeof(out->model), "MH-Z19");
    fill_timestamp(out->timestamp, sizeof(out->timestamp));

    // co stores CO2 ppm from MH-Z19 (or mock fallback).
    out->pm25 = 0.0f;
    out->pm10 = 0.0f;
    out->co = 400.0f + (float)(rand() % 2600);
#ifndef _WIN32
    float real_ppm = 0.0f;
    if (read_mhz19_ppm(&real_ppm) == 0)
        out->co = real_ppm;
#endif
    out->no2 = 0.0f;
    out->o3 = 0.0f;
    out->so2 = 0.0f;
    return 0;
}

static int mhz19_shutdown(void) {
    printf("MH-Z19 plugin shutdown.\n");
    return 0;
}

SENSOR_PLUGIN_EXPORT SensorPlugin sensor_plugin = {
    .api_version = SENSOR_PLUGIN_API_VERSION,
    .name = "MH-Z19",
    .plugin_version = "1.0.0",
    .description = "Simulated MH-Z19 CO2 ppm readings mapped to co field.",
    .sensor_id = 1900,
    .init = mhz19_init,
    .read_sample = mhz19_read_sample,
    .shutdown = mhz19_shutdown,
};

