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
    const struct tm *ptm = localtime(&now);
    if (!ptm) {
        snprintf(buf, len, "1970-01-01 00:00:00");
        return;
    }
    if (strftime(buf, len, "%Y-%m-%d %H:%M:%S", ptm) == 0) {
        snprintf(buf, len, "1970-01-01 00:00:00");
    }
}

static int pms5003_init(void) {
#ifndef _WIN32
    printf("PMS5003 plugin initialized (real UART on Linux/macOS if available; fallback mock).\n");
#else
    printf("PMS5003 plugin initialized (mock on Windows).\n");
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
    tty.c_cc[VMIN] = 1;
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

// parse and validate a 32-byte PMS5003 frame (bytes already include the
// 0x42 0x4D header). Pure function -- no I/O -- so it can be unit tested
// without a real UART connection.
// returns 0 on success (pm25/pm10 populated), -1 if the checksum fails.
int pms5003_parse_frame(const unsigned char frame[32], int *pm25, int *pm10) {
    if (!frame || !pm25 || !pm10)
        return -1;

    unsigned int sum = 0;
    for (int k = 0; k < 30; k++)
        sum += frame[k];
    unsigned int chk = ((unsigned int)frame[30] << 8) | frame[31];
    if ((sum & 0xFFFF) != chk)
        return -1;

    *pm25 = ((int)frame[12] << 8) | frame[13];
    *pm10 = ((int)frame[14] << 8) | frame[15];
    return 0;
}

static int read_frame(int fd, unsigned char *buf, size_t want) {
    size_t got = 0;
    while (got < want) {
        ssize_t n = read(fd, buf + got, want - got);
        if (n <= 0)
            return -1;
        got += (size_t)n;
    }
    return 0;
}

static int read_pms5003(float *pm25, float *pm10) {
    const char *dev = getenv("PMS5003_DEVICE");
    if (!dev || !dev[0])
        dev = "/dev/ttyUSB0";
    int fd = open_serial_9600(dev);
    if (fd < 0)
        return -1;

    unsigned char b;
    int found = 0;
    for (int i = 0; i < 2048; i++) {
        if (read(fd, &b, 1) != 1)
            break;
        if (!found && b == 0x42) {
            found = 1;
            continue;
        }
        if (found && b == 0x4d) {
            unsigned char frame[32];
            frame[0] = 0x42;
            frame[1] = 0x4d;
            if (read_frame(fd, frame + 2, 30) != 0) {
                close(fd);
                return -1;
            }
            int val_pm25 = 0, val_pm10 = 0;
            if (pms5003_parse_frame(frame, &val_pm25, &val_pm10) != 0) {
                close(fd);
                return -1;
            }
            *pm25 = (float)val_pm25;
            *pm10 = (float)val_pm10;
            close(fd);
            return 0;
        }
        found = 0;
    }
    close(fd);
    return -1;
}
#endif

static int pms5003_read_sample(AirQualityData *out) {
    if (!out)
        return -1;
    memset(out, 0, sizeof(*out));
    out->sensor_id = 5003;
    snprintf(out->model, sizeof(out->model), "PMS5003");
    fill_timestamp(out->timestamp, sizeof(out->timestamp));

    // PMS5003 particulate readings (real UART if available; mock fallback).
    out->pm25 = (float)(rand() % 5000) / 100.0f;          // 0..49.99
    out->pm10 = out->pm25 + (float)(rand() % 7000) / 100.0f; // PM10 >= PM2.5
#ifndef _WIN32
    float real_pm25 = 0.0f, real_pm10 = 0.0f;
    if (read_pms5003(&real_pm25, &real_pm10) == 0) {
        out->pm25 = real_pm25;
        out->pm10 = real_pm10;
    }
#endif
    out->co = 0.0f;
    out->no2 = 0.0f;
    out->o3 = 0.0f;
    out->so2 = 0.0f;
    return 0;
}

static int pms5003_shutdown(void) {
    printf("PMS5003 plugin shutdown.\n");
    return 0;
}

SENSOR_PLUGIN_EXPORT SensorPlugin sensor_plugin = {
    .api_version = SENSOR_PLUGIN_API_VERSION,
    .name = "PMS5003",
    .plugin_version = "1.0.0",
    .description = "Simulated PMS5003 particulate readings.",
    .sensor_id = 5003,
    .init = pms5003_init,
    .read_sample = pms5003_read_sample,
    .shutdown = pms5003_shutdown,
};