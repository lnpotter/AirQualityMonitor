#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALERT_EMAIL "your_email@example.com"

typedef struct {
    int sensor_id;
    float pm25;
    float pm10;
    float co;
    float no2;
    float o3;
    float so2;
} AirQualityData;

void check_alerts(AirQualityData data);
void send_email_alert(AirQualityData data);

void check_alerts(AirQualityData data) {
    if (data.pm25 > limit_pm25 || data.pm10 > limit_pm10 || data.co > limit_co || 
        data.no2 > limit_no2 || data.o3 > limit_o3 || data.so2 > limit_so2) {
        printf("ALERT: Dangerous pollutant levels detected!\n");
        send_email_alert(data);
    } else {
        printf("Air quality within safe limits.\n");
    }
}

void send_email_alert(AirQualityData data) {
    char command[1024];
    snprintf(command, sizeof(command),
             "echo 'ALERT: Dangerous pollutant levels detected!\n\n"
             "PM2.5: %.2f\nPM10: %.2f\nCO: %.2f\nNO2: %.2f\nO3: %.2f\nSO2: %.2f\n' | mail -s 'Air Quality Alert' %s",
             data.pm25, data.pm10, data.co, data.no2, data.o3, data.so2, ALERT_EMAIL);
    system(command);
}
