#include "core/aqm_db.h"
#include "core/globals.h"
#include <stdio.h>
#include <sqlite3.h>
#include <time.h>
#include <string.h>

#define CSV_FILE "sensor_data.csv"

typedef struct {
    int sensor_id;
    char name[64];
    char model[64];
    double pm25_sum, pm10_sum, co_sum, no2_sum, o3_sum, so2_sum;
    double pm25_min, pm10_min, co_min, no2_min, o3_min, so2_min;
    double pm25_max, pm10_max, co_max, no2_max, o3_max, so2_max;
    int count;
    int alerts_pm25, alerts_pm10, alerts_co, alerts_no2, alerts_o3, alerts_so2;
} SensorStats;

static void init_sensor_stats(SensorStats *stats, int sensor_id, const char *name, const char *model) {
    stats->sensor_id = sensor_id;
    snprintf(stats->name, sizeof(stats->name), "%s", name ? name : "");
    snprintf(stats->model, sizeof(stats->model), "%s", model ? model : "");
    stats->pm25_sum = stats->pm10_sum = stats->co_sum = stats->no2_sum = stats->o3_sum = stats->so2_sum = 0.0;
    stats->pm25_min = stats->pm10_min = stats->co_min = stats->no2_min = stats->o3_min = stats->so2_min = 999999.0;
    stats->pm25_max = stats->pm10_max = stats->co_max = stats->no2_max = stats->o3_max = stats->so2_max = 0.0;
    stats->count = 0;
    stats->alerts_pm25 = stats->alerts_pm10 = stats->alerts_co = stats->alerts_no2 = stats->alerts_o3 = stats->alerts_so2 = 0;
}

static void update_sensor_stats(SensorStats *stats, double pm25, double pm10, double co, double no2, double o3, double so2) {
    stats->count++;
    stats->pm25_sum += pm25; stats->pm10_sum += pm10; stats->co_sum += co;
    stats->no2_sum += no2; stats->o3_sum += o3; stats->so2_sum += so2;
    if (pm25 < stats->pm25_min) stats->pm25_min = pm25;
    if (pm10 < stats->pm10_min) stats->pm10_min = pm10;
    if (co < stats->co_min) stats->co_min = co;
    if (no2 < stats->no2_min) stats->no2_min = no2;
    if (o3 < stats->o3_min) stats->o3_min = o3;
    if (so2 < stats->so2_min) stats->so2_min = so2;
    if (pm25 > stats->pm25_max) stats->pm25_max = pm25;
    if (pm10 > stats->pm10_max) stats->pm10_max = pm10;
    if (co > stats->co_max) stats->co_max = co;
    if (no2 > stats->no2_max) stats->no2_max = no2;
    if (o3 > stats->o3_max) stats->o3_max = o3;
    if (so2 > stats->so2_max) stats->so2_max = so2;
    if (pm25 > limit_pm25) stats->alerts_pm25++;
    if (pm10 > limit_pm10) stats->alerts_pm10++;
    if (co > limit_co) stats->alerts_co++;
    if (no2 > limit_no2) stats->alerts_no2++;
    if (o3 > limit_o3) stats->alerts_o3++;
    if (so2 > limit_so2) stats->alerts_so2++;
}

static void write_csv_header(FILE *csv_file, int total_records, const char *period_start, const char *period_end) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char export_time[32];
    strftime(export_time, sizeof(export_time), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(csv_file, "# Air Quality Monitor - Data Export Report\n");
    fprintf(csv_file, "# Export Date: %s\n", export_time);
    fprintf(csv_file, "# Data Period: %s to %s\n", period_start && period_start[0] ? period_start : "N/A", period_end && period_end[0] ? period_end : "N/A");
    fprintf(csv_file, "# Total Records: %d\n", total_records);
    fprintf(csv_file, "# Pollutant Limits: PM2.5=%.2f, PM10=%.2f, CO=%.2f, NO2=%.4f, O3=%.4f, SO2=%.4f\n",
            limit_pm25, limit_pm10, limit_co, limit_no2, limit_o3, limit_so2);
    fprintf(csv_file, "#\n");
}

static void write_sensor_section_header(FILE *csv_file, SensorStats *stats) {
    fprintf(csv_file, "#\n");
    fprintf(csv_file, "# ============================================\n");
    fprintf(csv_file, "# SENSOR: %s (ID: %d, Model: %s)\n", stats->name, stats->sensor_id, stats->model);
    fprintf(csv_file, "# Records: %d\n", stats->count);
    fprintf(csv_file, "# ============================================\n");
    fprintf(csv_file, "id,sensor_id,sensor_name,model,measured_at,pm25,pm10,co,no2,o3,so2,status,alert_details\n");
}

static void write_sensor_reading(FILE *csv_file, sqlite3_stmt *res, int *alert_count) {
    int id = sqlite3_column_int(res, 0);
    int sensor_id = sqlite3_column_int(res, 1);
    const char *sname = (const char *)sqlite3_column_text(res, 2);
    const char *model = (const char *)sqlite3_column_text(res, 3);
    const char *mts = (const char *)sqlite3_column_text(res, 4);
    double pm25 = sqlite3_column_double(res, 5);
    double pm10 = sqlite3_column_double(res, 6);
    double co = sqlite3_column_double(res, 7);
    double no2 = sqlite3_column_double(res, 8);
    double o3 = sqlite3_column_double(res, 9);
    double so2 = sqlite3_column_double(res, 10);
    
    if (!sname) sname = "";
    if (!mts) mts = "";
    if (!model) model = "";
    
    // Determine status and alert details
    char status[16] = "OK";
    char alert_details[256] = "";
    int has_alert = 0;
    
    if (pm25 > limit_pm25) {
        has_alert = 1;
        strcat(alert_details, "PM2.5 ");
    }
    if (pm10 > limit_pm10) {
        has_alert = 1;
        strcat(alert_details, "PM10 ");
    }
    if (co > limit_co) {
        has_alert = 1;
        strcat(alert_details, "CO ");
    }
    if (no2 > limit_no2) {
        has_alert = 1;
        strcat(alert_details, "NO2 ");
    }
    if (o3 > limit_o3) {
        has_alert = 1;
        strcat(alert_details, "O3 ");
    }
    if (so2 > limit_so2) {
        has_alert = 1;
        strcat(alert_details, "SO2 ");
    }
    
    if (has_alert) {
        strcpy(status, "ALERT");
        (*alert_count)++;
    }
    
    // Trim trailing space from alert_details
    int len = strlen(alert_details);
    if (len > 0 && alert_details[len-1] == ' ') {
        alert_details[len-1] = '\0';
    }
    
    fprintf(csv_file, "%d,%d,\"%s\",\"%s\",%s,%.4f,%.4f,%.4f,%.6f,%.6f,%.6f,%s,\"%s\"\n",
            id, sensor_id, sname, model, mts, pm25, pm10, co, no2, o3, so2, status, alert_details);
}

static void write_sensor_statistics(FILE *csv_file, SensorStats *stats) {
    if (stats->count == 0) return;
    
    fprintf(csv_file, "#\n");
    fprintf(csv_file, "# STATISTICS FOR %s:\n", stats->name);
    fprintf(csv_file, "# Pollutant,Min,Avg,Max,Alert_Count\n");
    fprintf(csv_file, "# PM2.5,%.4f,%.4f,%.4f,%d\n", stats->pm25_min, stats->pm25_sum/stats->count, stats->pm25_max, stats->alerts_pm25);
    fprintf(csv_file, "# PM10,%.4f,%.4f,%.4f,%d\n", stats->pm10_min, stats->pm10_sum/stats->count, stats->pm10_max, stats->alerts_pm10);
    fprintf(csv_file, "# CO,%.4f,%.4f,%.4f,%d\n", stats->co_min, stats->co_sum/stats->count, stats->co_max, stats->alerts_co);
    fprintf(csv_file, "# NO2,%.6f,%.6f,%.6f,%d\n", stats->no2_min, stats->no2_sum/stats->count, stats->no2_max, stats->alerts_no2);
    fprintf(csv_file, "# O3,%.6f,%.6f,%.6f,%d\n", stats->o3_min, stats->o3_sum/stats->count, stats->o3_max, stats->alerts_o3);
    fprintf(csv_file, "# SO2,%.6f,%.6f,%.6f,%d\n", stats->so2_min, stats->so2_sum/stats->count, stats->so2_max, stats->alerts_so2);
    fprintf(csv_file, "# Total Alerts: %d\n", stats->alerts_pm25 + stats->alerts_pm10 + stats->alerts_co + stats->alerts_no2 + stats->alerts_o3 + stats->alerts_so2);
}

static void write_global_summary(FILE *csv_file, SensorStats *stats, int sensor_count, int total_records, int total_alerts) {
    fprintf(csv_file, "#\n");
    fprintf(csv_file, "# ============================================\n");
    fprintf(csv_file, "# GLOBAL SUMMARY\n");
    fprintf(csv_file, "# ============================================\n");
    fprintf(csv_file, "# Total Sensors: %d\n", sensor_count);
    fprintf(csv_file, "# Total Records: %d\n", total_records);
    fprintf(csv_file, "# Total Alert Events: %d\n", total_alerts);
    fprintf(csv_file, "#\n");
    fprintf(csv_file, "# Alert Summary by Sensor:\n");
    fprintf(csv_file, "# Sensor,PM2.5,PM10,CO,NO2,O3,SO2,Total\n");
    
    for (int i = 0; i < sensor_count; i++) {
        int total = stats[i].alerts_pm25 + stats[i].alerts_pm10 + stats[i].alerts_co + 
                    stats[i].alerts_no2 + stats[i].alerts_o3 + stats[i].alerts_so2;
        fprintf(csv_file, "# %s,%d,%d,%d,%d,%d,%d,%d\n", 
                stats[i].name, stats[i].alerts_pm25, stats[i].alerts_pm10, stats[i].alerts_co,
                stats[i].alerts_no2, stats[i].alerts_o3, stats[i].alerts_so2, total);
    }
}

void export_to_csv(void) {
    sqlite3 *db = NULL;
    if (aqm_db_open(&db) != 0)
        return;

    // First, get period range and count
    sqlite3_stmt *count_res = NULL;
    const char *count_sql = "SELECT COUNT(*), MIN(measured_at), MAX(measured_at) FROM readings;";
    int total_records = 0;
    char period_start[32] = "";
    char period_end[32] = "";
    
    if (sqlite3_prepare_v2(db, count_sql, -1, &count_res, NULL) == SQLITE_OK) {
        if (sqlite3_step(count_res) == SQLITE_ROW) {
            total_records = sqlite3_column_int(count_res, 0);
            const char *min_date = (const char *)sqlite3_column_text(count_res, 1);
            const char *max_date = (const char *)sqlite3_column_text(count_res, 2);
            if (min_date) snprintf(period_start, sizeof(period_start), "%s", min_date);
            if (max_date) snprintf(period_end, sizeof(period_end), "%s", max_date);
        }
        sqlite3_finalize(count_res);
    }

    FILE *csv_file = fopen(CSV_FILE, "w");
    if (!csv_file) {
        fprintf(stderr, "Cannot open CSV file for writing: %s\n", CSV_FILE);
        aqm_db_close(db);
        return;
    }

    // Write header
    write_csv_header(csv_file, total_records, period_start, period_end);

    // Get unique sensors
    sqlite3_stmt *sensor_res = NULL;
    const char *sensor_sql = "SELECT DISTINCT s.id, s.name FROM readings r JOIN sensors s ON s.id = r.sensor_id ORDER BY s.id;";
    SensorStats sensor_stats[50];
    int sensor_count = 0;
    int total_alerts = 0;

    if (sqlite3_prepare_v2(db, sensor_sql, -1, &sensor_res, NULL) == SQLITE_OK) {
        while (sqlite3_step(sensor_res) == SQLITE_ROW && sensor_count < 50) {
            int sid = sqlite3_column_int(sensor_res, 0);
            const char *sname = (const char *)sqlite3_column_text(sensor_res, 1);
            init_sensor_stats(&sensor_stats[sensor_count], sid, sname ? sname : "", "");
            sensor_count++;
        }
        sqlite3_finalize(sensor_res);
    }

    // Process each sensor
    sqlite3_stmt *res = NULL;
    const char *sql =
        "SELECT r.id, r.sensor_id, s.name, r.model, r.measured_at, r.pm25, r.pm10, r.co, r.no2, r.o3, r.so2 "
        "FROM readings r JOIN sensors s ON s.id = r.sensor_id "
        "WHERE r.sensor_id = ? "
        "ORDER BY r.measured_at ASC;";

    for (int i = 0; i < sensor_count; i++) {
        if (sqlite3_prepare_v2(db, sql, -1, &res, NULL) != SQLITE_OK) {
            continue;
        }
        sqlite3_bind_int(res, 1, sensor_stats[i].sensor_id);
        
        int first = 1;
        while (sqlite3_step(res) == SQLITE_ROW) {
            if (first) {
                // Update model name from first record
                const char *model = (const char *)sqlite3_column_text(res, 3);
                if (model) snprintf(sensor_stats[i].model, sizeof(sensor_stats[i].model), "%s", model);
                write_sensor_section_header(csv_file, &sensor_stats[i]);
                first = 0;
            }
            
            double pm25 = sqlite3_column_double(res, 5);
            double pm10 = sqlite3_column_double(res, 6);
            double co = sqlite3_column_double(res, 7);
            double no2 = sqlite3_column_double(res, 8);
            double o3 = sqlite3_column_double(res, 9);
            double so2 = sqlite3_column_double(res, 10);
            
            update_sensor_stats(&sensor_stats[i], pm25, pm10, co, no2, o3, so2);
            write_sensor_reading(csv_file, res, &total_alerts);
        }
        
        if (!first) {
            write_sensor_statistics(csv_file, &sensor_stats[i]);
        }
        
        sqlite3_finalize(res);
        res = NULL;
    }

    // Write global summary
    write_global_summary(csv_file, sensor_stats, sensor_count, total_records, total_alerts);

    printf("Data exported to %s successfully.\n", CSV_FILE);
    printf("Total records: %d | Sensors: %d | Alert events: %d\n", total_records, sensor_count, total_alerts);

    aqm_db_close(db);
    fclose(csv_file);
}
