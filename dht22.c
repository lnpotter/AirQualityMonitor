#include "sensor.h"
#include <wiringPi.h>
#include <stdio.h>

#define DHT_PIN 7  // GPIO Pin

static int dht22_data[5] = {0, 0, 0, 0, 0};

static float read_data() {
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t j = 0, i;

    dht22_data[0] = dht22_data[1] = dht22_data[2] = dht22_data[3] = dht22_data[4] = 0;

    pinMode(DHT_PIN, OUTPUT);
    digitalWrite(DHT_PIN, LOW);
    delay(18);
    digitalWrite(DHT_PIN, HIGH);
    delayMicroseconds(40);
    pinMode(DHT_PIN, INPUT);

    for (i = 0; i < MAX_TIMINGS; i++) {
        counter = 0;
        while (digitalRead(DHT_PIN) == laststate) {
            counter++;
            delayMicroseconds(1);
            if (counter == 255) break;
        }
        laststate = digitalRead(DHT_PIN);
        if (counter == 255) break;
        if ((i >= 4) && (i % 2 == 0)) {
            dht22_data[j / 8] <<= 1;
            if (counter > 16) dht22_data[j / 8] |= 1;
            j++;
        }
    }

    if ((j >= 40) && (dht22_data[4] == ((dht22_data[0] + dht22_data[1] + dht22_data[2] + dht22_data[3]) & 0xFF))) {
        float h = (float)((dht22_data[0] << 8) + dht22_data[1]) / 10;
        if (h > 100) h = dht22_data[0];
        float c = (float)(((dht22_data[2] & 0x7F) << 8) + dht22_data[3]) / 10;
        if (c > 125) c = dht22_data[2];
        if (dht22_data[2] & 0x80) c = -c;
        printf("DHT22: Humidity = %.1f %% Temperature = %.1f *C\n", h, c);
        return c;  // Return temperature as an example metric
    } else {
        printf("DHT22: Data not good, skip\n");
        return -1;
    }
}

static int init() {
    if (wiringPiSetup() == -1) {
        printf("DHT22: WiringPi setup failed\n");
        return -1;
    }
    printf("DHT22 sensor initialized.\n");
    return 0;
}

static int shutdown() {
    printf("DHT22 sensor shutdown.\n");
    return 0;
}

Sensor sensor = {
    .name = "DHT22",
    .read_data = read_data,
    .init = init,
    .shutdown = shutdown
};
