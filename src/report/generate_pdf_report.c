#include "core/aqm_db.h"
#include "core/globals.h"
#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <time.h>

#ifdef HAVE_HPDF
#include <hpdf.h>

#define PDF_FILE "sensor_data_report.pdf"
#define MAX_SENSORS 50
#define MAX_ALERTS 200

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

typedef struct {
    int sensor_id;
    char sensor_name[64];
    char timestamp[32];
    char pollutant[16];
    double value;
    double limit;
} AlertRecord;

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

static void add_alert(AlertRecord *alerts, int *alert_count, int sensor_id, const char *sensor_name, 
                      const char *timestamp, const char *pollutant, double value, double limit) {
    if (*alert_count >= MAX_ALERTS) return;
    alerts[*alert_count].sensor_id = sensor_id;
    snprintf(alerts[*alert_count].sensor_name, sizeof(alerts[*alert_count].sensor_name), "%s", sensor_name);
    snprintf(alerts[*alert_count].timestamp, sizeof(alerts[*alert_count].timestamp), "%s", timestamp);
    snprintf(alerts[*alert_count].pollutant, sizeof(alerts[*alert_count].pollutant), "%s", pollutant);
    alerts[*alert_count].value = value;
    alerts[*alert_count].limit = limit;
    (*alert_count)++;
}

static HPDF_Page create_new_page(HPDF_Doc pdf, HPDF_Font font, int font_size) {
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    HPDF_Page_SetFontAndSize(page, font, font_size);
    return page;
}

static void draw_footer(HPDF_Page page, int page_num, HPDF_Font font) {
    HPDF_Page_SetFontAndSize(page, font, 8);
    char footer[64];
    snprintf(footer, sizeof(footer), "Page %d", page_num);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 40, 30, footer);
    HPDF_Page_TextOut(page, 400, 30, "Air Quality Monitor Report");
    HPDF_Page_EndText(page);
}

static void draw_cover_page(HPDF_Doc pdf, HPDF_Font font_bold, HPDF_Font font, 
                            int total_sensors, int total_records, int total_alerts,
                            const char *period_start, const char *period_end) {
    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    
    HPDF_REAL page_width = HPDF_Page_GetWidth(page);
    HPDF_REAL center_x = page_width / 2;
    
    // Title
    HPDF_Page_SetFontAndSize(page, font_bold, 24);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, center_x - 140, 700, "AIR QUALITY MONITOR");
    HPDF_Page_EndText(page);
    
    HPDF_Page_SetFontAndSize(page, font_bold, 18);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, center_x - 100, 660, "Data Report");
    HPDF_Page_EndText(page);
    
    // Date
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char date_str[64];
    strftime(date_str, sizeof(date_str), "Generated: %Y-%m-%d %H:%M", tm_info);
    HPDF_Page_SetFontAndSize(page, font, 12);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, center_x - 80, 600, date_str);
    HPDF_Page_EndText(page);
    
    // Data period
    HPDF_Page_BeginText(page);
    char period_str[128];
    snprintf(period_str, sizeof(period_str), "Data Period: %s to %s", 
             period_start && period_start[0] ? period_start : "N/A",
             period_end && period_end[0] ? period_end : "N/A");
    HPDF_Page_TextOut(page, 100, 550, period_str);
    HPDF_Page_EndText(page);
    
    // Summary box
    HPDF_Page_SetRGBStroke(page, 0.3, 0.3, 0.3);
    HPDF_Page_SetRGBFill(page, 0.95, 0.95, 0.95);
    HPDF_Page_Rectangle(page, 80, 420, 450, 100);
    HPDF_Page_FillStroke(page);
    
    HPDF_Page_SetFontAndSize(page, font_bold, 14);
    HPDF_Page_SetRGBFill(page, 0, 0, 0);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, center_x - 50, 490, "SUMMARY");
    HPDF_Page_EndText(page);
    
    HPDF_Page_SetFontAndSize(page, font, 12);
    HPDF_Page_BeginText(page);
    char summary[256];
    snprintf(summary, sizeof(summary), "Total Sensors: %d", total_sensors);
    HPDF_Page_TextOut(page, 100, 460, summary);
    snprintf(summary, sizeof(summary), "Total Records: %d", total_records);
    HPDF_Page_TextOut(page, 100, 440, summary);
    snprintf(summary, sizeof(summary), "Total Alert Events: %d", total_alerts);
    HPDF_Page_TextOut(page, 100, 420, summary);
    HPDF_Page_EndText(page);
}

static void draw_summary_page(HPDF_Doc pdf, HPDF_Font font_bold, HPDF_Font font, 
                              SensorStats *stats, int sensor_count, int *page_num) {
    HPDF_Page page = create_new_page(pdf, font, 10);
    (*page_num)++;
    
    HPDF_Page_SetFontAndSize(page, font_bold, 16);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 40, 780, "Global Statistics Overview");
    HPDF_Page_EndText(page);
    
    HPDF_REAL y = 740;
    HPDF_Page_SetFontAndSize(page, font, 9);
    
    // Table header
    HPDF_Page_SetRGBFill(page, 0.8, 0.8, 0.8);
    HPDF_Page_Rectangle(page, 40, y - 5, 520, 20);
    HPDF_Page_FillStroke(page);
    HPDF_Page_SetRGBFill(page, 0, 0, 0);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 45, y, "Sensor");
    HPDF_Page_TextOut(page, 150, y, "Count");
    HPDF_Page_TextOut(page, 200, y, "PM2.5 Avg");
    HPDF_Page_TextOut(page, 280, y, "PM10 Avg");
    HPDF_Page_TextOut(page, 350, y, "CO Avg");
    HPDF_Page_TextOut(page, 420, y, "Alerts");
    HPDF_Page_EndText(page);
    y -= 25;
    
    for (int i = 0; i < sensor_count && y > 100; i++) {
        if (stats[i].count == 0) continue;
        
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, 45, y, stats[i].name);
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", stats[i].count);
        HPDF_Page_TextOut(page, 150, y, buf);
        snprintf(buf, sizeof(buf), "%.2f", stats[i].pm25_sum / stats[i].count);
        HPDF_Page_TextOut(page, 200, y, buf);
        snprintf(buf, sizeof(buf), "%.2f", stats[i].pm10_sum / stats[i].count);
        HPDF_Page_TextOut(page, 280, y, buf);
        snprintf(buf, sizeof(buf), "%.2f", stats[i].co_sum / stats[i].count);
        HPDF_Page_TextOut(page, 350, y, buf);
        int total_alerts = stats[i].alerts_pm25 + stats[i].alerts_pm10 + stats[i].alerts_co + 
                          stats[i].alerts_no2 + stats[i].alerts_o3 + stats[i].alerts_so2;
        snprintf(buf, sizeof(buf), "%d", total_alerts);
        HPDF_Page_TextOut(page, 420, y, buf);
        HPDF_Page_EndText(page);
        y -= 15;
    }
    
    draw_footer(page, *page_num, font);
}

static void draw_bar_chart(HPDF_Page page, HPDF_Font font, float x, float y, 
                           const char *label, double value, double max_val, int is_alert) {
    float bar_width = 200;
    float bar_height = 15;
    float filled_width = (max_val > 0) ? (value / max_val) * bar_width : 0;
    
    // Background bar
    HPDF_Page_SetRGBStroke(page, 0.7, 0.7, 0.7);
    HPDF_Page_SetRGBFill(page, 0.9, 0.9, 0.9);
    HPDF_Page_Rectangle(page, x, y, bar_width, bar_height);
    HPDF_Page_FillStroke(page);
    
    // Filled portion
    if (is_alert) {
        HPDF_Page_SetRGBFill(page, 0.9, 0.3, 0.3); // Red for alerts
    } else {
        HPDF_Page_SetRGBFill(page, 0.3, 0.6, 0.9); // Blue for normal
    }
    HPDF_Page_Rectangle(page, x, y, filled_width, bar_height);
    HPDF_Page_FillStroke(page);
    
    // Label and value
    HPDF_Page_SetFontAndSize(page, font, 8);
    HPDF_Page_SetRGBFill(page, 0, 0, 0);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, x, y + bar_height + 2, label);
    char val_str[32];
    snprintf(val_str, sizeof(val_str), "%.2f", value);
    HPDF_Page_TextOut(page, x + bar_width + 5, y + 3, val_str);
    HPDF_Page_EndText(page);
}

static void draw_sensor_detail_page(HPDF_Doc pdf, HPDF_Font font_bold, HPDF_Font font,
                                    SensorStats *stats, sqlite3 *db, int *page_num) {
    if (stats->count == 0) return;
    
    HPDF_Page page = create_new_page(pdf, font, 10);
    (*page_num)++;
    
    // Header
    HPDF_Page_SetFontAndSize(page, font_bold, 16);
    HPDF_Page_SetRGBFill(page, 0.2, 0.3, 0.5);
    HPDF_Page_BeginText(page);
    char header[128];
    snprintf(header, sizeof(header), "Sensor: %s (ID: %d)", stats->name, stats->sensor_id);
    HPDF_Page_TextOut(page, 40, 780, header);
    HPDF_Page_EndText(page);
    
    HPDF_Page_SetFontAndSize(page, font, 11);
    HPDF_Page_SetRGBFill(page, 0, 0, 0);
    HPDF_Page_BeginText(page);
    snprintf(header, sizeof(header), "Model: %s | Records: %d", stats->model, stats->count);
    HPDF_Page_TextOut(page, 40, 760, header);
    HPDF_Page_EndText(page);
    
    // Statistics table
    HPDF_REAL y = 720;
    HPDF_Page_SetFontAndSize(page, font_bold, 12);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 40, y, "Statistics Summary");
    HPDF_Page_EndText(page);
    y -= 25;
    
    // Statistics table header
    HPDF_Page_SetRGBFill(page, 0.85, 0.85, 0.85);
    HPDF_Page_Rectangle(page, 40, y - 5, 520, 18);
    HPDF_Page_FillStroke(page);
    HPDF_Page_SetRGBFill(page, 0, 0, 0);
    HPDF_Page_SetFontAndSize(page, font_bold, 9);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 45, y, "Pollutant");
    HPDF_Page_TextOut(page, 130, y, "Minimum");
    HPDF_Page_TextOut(page, 210, y, "Average");
    HPDF_Page_TextOut(page, 290, y, "Maximum");
    HPDF_Page_TextOut(page, 370, y, "Limit");
    HPDF_Page_TextOut(page, 450, y, "Alerts");
    HPDF_Page_EndText(page);
    y -= 20;
    
    HPDF_Page_SetFontAndSize(page, font, 9);
    
    struct { const char *name; double min, avg, max, limit; int alerts; } pollutants[6] = {
        {"PM2.5", stats->pm25_min, stats->pm25_sum/stats->count, stats->pm25_max, limit_pm25, stats->alerts_pm25},
        {"PM10", stats->pm10_min, stats->pm10_sum/stats->count, stats->pm10_max, limit_pm10, stats->alerts_pm10},
        {"CO", stats->co_min, stats->co_sum/stats->count, stats->co_max, limit_co, stats->alerts_co},
        {"NO2", stats->no2_min, stats->no2_sum/stats->count, stats->no2_max, limit_no2, stats->alerts_no2},
        {"O3", stats->o3_min, stats->o3_sum/stats->count, stats->o3_max, limit_o3, stats->alerts_o3},
        {"SO2", stats->so2_min, stats->so2_sum/stats->count, stats->so2_max, limit_so2, stats->alerts_so2}
    };
    
    for (int i = 0; i < 6; i++) {
        // Highlight if there are alerts
        if (pollutants[i].alerts > 0) {
            HPDF_Page_SetRGBFill(page, 1.0, 0.9, 0.9);
            HPDF_Page_Rectangle(page, 40, y - 3, 520, 15);
            HPDF_Page_FillStroke(page);
        }
        
        HPDF_Page_SetRGBFill(page, 0, 0, 0);
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, 45, y, pollutants[i].name);
        char buf[32];
        snprintf(buf, sizeof(buf), "%.4f", pollutants[i].min);
        HPDF_Page_TextOut(page, 130, y, buf);
        snprintf(buf, sizeof(buf), "%.4f", pollutants[i].avg);
        HPDF_Page_TextOut(page, 210, y, buf);
        snprintf(buf, sizeof(buf), "%.4f", pollutants[i].max);
        HPDF_Page_TextOut(page, 290, y, buf);
        snprintf(buf, sizeof(buf), "%.4f", pollutants[i].limit);
        HPDF_Page_TextOut(page, 370, y, buf);
        snprintf(buf, sizeof(buf), "%d", pollutants[i].alerts);
        HPDF_Page_TextOut(page, 450, y, buf);
        HPDF_Page_EndText(page);
        y -= 16;
    }
    
    // Bar charts section
    y -= 20;
    HPDF_Page_SetFontAndSize(page, font_bold, 12);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 40, y, "Average Values Chart");
    HPDF_Page_EndText(page);
    y -= 30;
    
    double max_val = 0;
    for (int i = 0; i < 6; i++) {
        if (pollutants[i].avg > max_val) max_val = pollutants[i].avg;
    }
    if (max_val == 0) max_val = 1;
    
    for (int i = 0; i < 6 && y > 100; i++) {
        draw_bar_chart(page, font, 60, y, pollutants[i].name, pollutants[i].avg, max_val, pollutants[i].alerts > 0);
        y -= 35;
    }
    
    // Recent readings table
    if (y < 250) {
        draw_footer(page, *page_num, font);
        page = create_new_page(pdf, font, 9);
        (*page_num)++;
        y = 760;
    } else {
        y -= 30;
    }
    
    HPDF_Page_SetFontAndSize(page, font_bold, 12);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 40, y, "Recent Readings");
    HPDF_Page_EndText(page);
    y -= 25;
    
    // Readings table header
    HPDF_Page_SetRGBFill(page, 0.85, 0.85, 0.85);
    HPDF_Page_Rectangle(page, 40, y - 5, 520, 18);
    HPDF_Page_FillStroke(page);
    HPDF_Page_SetRGBFill(page, 0, 0, 0);
    HPDF_Page_SetFontAndSize(page, font_bold, 8);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 45, y, "Time");
    HPDF_Page_TextOut(page, 180, y, "PM2.5");
    HPDF_Page_TextOut(page, 240, y, "PM10");
    HPDF_Page_TextOut(page, 290, y, "CO");
    HPDF_Page_TextOut(page, 350, y, "NO2");
    HPDF_Page_TextOut(page, 410, y, "O3");
    HPDF_Page_TextOut(page, 470, y, "SO2");
    HPDF_Page_EndText(page);
    y -= 18;
    
    // Query recent readings for this sensor
    sqlite3_stmt *res = NULL;
    const char *sql = "SELECT measured_at, pm25, pm10, co, no2, o3, so2 FROM readings "
                      "WHERE sensor_id = ? ORDER BY measured_at DESC LIMIT 20;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &res, NULL) == SQLITE_OK) {
        sqlite3_bind_int(res, 1, stats->sensor_id);
        HPDF_Page_SetFontAndSize(page, font, 8);
        
        int row_count = 0;
        while (sqlite3_step(res) == SQLITE_ROW && y > 100 && row_count < 20) {
            const char *time = (const char *)sqlite3_column_text(res, 0);
            double pm25 = sqlite3_column_double(res, 1);
            double pm10 = sqlite3_column_double(res, 2);
            double co = sqlite3_column_double(res, 3);
            double no2 = sqlite3_column_double(res, 4);
            double o3 = sqlite3_column_double(res, 5);
            double so2 = sqlite3_column_double(res, 6);
            
            // Check for alerts
            int has_alert = (pm25 > limit_pm25 || pm10 > limit_pm10 || co > limit_co ||
                            no2 > limit_no2 || o3 > limit_o3 || so2 > limit_so2);
            
            if (has_alert) {
                HPDF_Page_SetRGBFill(page, 1.0, 0.85, 0.85);
                HPDF_Page_Rectangle(page, 40, y - 3, 520, 12);
                HPDF_Page_FillStroke(page);
            }
            
            HPDF_Page_SetRGBFill(page, 0, 0, 0);
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, 45, y, time ? time : "");
            char buf[16];
            snprintf(buf, sizeof(buf), "%.2f", pm25);
            HPDF_Page_TextOut(page, 180, y, buf);
            snprintf(buf, sizeof(buf), "%.2f", pm10);
            HPDF_Page_TextOut(page, 240, y, buf);
            snprintf(buf, sizeof(buf), "%.2f", co);
            HPDF_Page_TextOut(page, 290, y, buf);
            snprintf(buf, sizeof(buf), "%.4f", no2);
            HPDF_Page_TextOut(page, 350, y, buf);
            snprintf(buf, sizeof(buf), "%.4f", o3);
            HPDF_Page_TextOut(page, 410, y, buf);
            snprintf(buf, sizeof(buf), "%.4f", so2);
            HPDF_Page_TextOut(page, 470, y, buf);
            HPDF_Page_EndText(page);
            
            y -= 13;
            row_count++;
        }
        sqlite3_finalize(res);
    }
    
    draw_footer(page, *page_num, font);
}

static void draw_alerts_page(HPDF_Doc pdf, HPDF_Font font_bold, HPDF_Font font,
                              AlertRecord *alerts, int alert_count, int *page_num) {
    if (alert_count == 0) return;
    
    HPDF_Page page = create_new_page(pdf, font, 10);
    (*page_num)++;
    
    HPDF_Page_SetFontAndSize(page, font_bold, 16);
    HPDF_Page_SetRGBFill(page, 0.8, 0.2, 0.2);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 40, 780, "Alert Summary");
    HPDF_Page_EndText(page);
    
    HPDF_Page_SetFontAndSize(page, font, 11);
    HPDF_Page_SetRGBFill(page, 0, 0, 0);
    HPDF_Page_BeginText(page);
    char buf[64];
    snprintf(buf, sizeof(buf), "Total Alert Events: %d", alert_count);
    HPDF_Page_TextOut(page, 40, 760, buf);
    HPDF_Page_EndText(page);
    
    HPDF_REAL y = 730;
    
    // Table header
    HPDF_Page_SetRGBFill(page, 0.9, 0.6, 0.6);
    HPDF_Page_Rectangle(page, 40, y - 5, 520, 20);
    HPDF_Page_FillStroke(page);
    HPDF_Page_SetRGBFill(page, 0, 0, 0);
    HPDF_Page_SetFontAndSize(page, font_bold, 9);
    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 45, y, "Time");
    HPDF_Page_TextOut(page, 180, y, "Sensor");
    HPDF_Page_TextOut(page, 280, y, "Pollutant");
    HPDF_Page_TextOut(page, 370, y, "Value");
    HPDF_Page_TextOut(page, 450, y, "Limit");
    HPDF_Page_EndText(page);
    y -= 25;
    
    HPDF_Page_SetFontAndSize(page, font, 9);
    
    for (int i = 0; i < alert_count && i < 50; i++) {
        if (y < 100) {
            draw_footer(page, *page_num, font);
            page = create_new_page(pdf, font, 9);
            (*page_num)++;
            y = 760;
            
            // Repeat header
            HPDF_Page_SetRGBFill(page, 0.9, 0.6, 0.6);
            HPDF_Page_Rectangle(page, 40, y - 5, 520, 20);
            HPDF_Page_FillStroke(page);
            HPDF_Page_SetRGBFill(page, 0, 0, 0);
            HPDF_Page_SetFontAndSize(page, font_bold, 9);
            HPDF_Page_BeginText(page);
            HPDF_Page_TextOut(page, 45, y, "Time");
            HPDF_Page_TextOut(page, 180, y, "Sensor");
            HPDF_Page_TextOut(page, 280, y, "Pollutant");
            HPDF_Page_TextOut(page, 370, y, "Value");
            HPDF_Page_TextOut(page, 450, y, "Limit");
            HPDF_Page_EndText(page);
            y -= 25;
            HPDF_Page_SetFontAndSize(page, font, 9);
        }
        
        HPDF_Page_BeginText(page);
        HPDF_Page_TextOut(page, 45, y, alerts[i].timestamp);
        HPDF_Page_TextOut(page, 180, y, alerts[i].sensor_name);
        HPDF_Page_TextOut(page, 280, y, alerts[i].pollutant);
        char val[32];
        snprintf(val, sizeof(val), "%.4f", alerts[i].value);
        HPDF_Page_TextOut(page, 370, y, val);
        snprintf(val, sizeof(val), "%.4f", alerts[i].limit);
        HPDF_Page_TextOut(page, 450, y, val);
        HPDF_Page_EndText(page);
        
        y -= 15;
    }
    
    draw_footer(page, *page_num, font);
}

void generate_pdf_report(void) {
    sqlite3 *db = NULL;
    if (aqm_db_open(&db) != 0)
        return;

    // Get data range and count
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

    // Get unique sensors
    sqlite3_stmt *sensor_res = NULL;
    const char *sensor_sql = "SELECT DISTINCT s.id, s.name FROM readings r JOIN sensors s ON s.id = r.sensor_id ORDER BY s.id;";
    SensorStats sensor_stats[MAX_SENSORS];
    int sensor_count = 0;
    
    if (sqlite3_prepare_v2(db, sensor_sql, -1, &sensor_res, NULL) == SQLITE_OK) {
        while (sqlite3_step(sensor_res) == SQLITE_ROW && sensor_count < MAX_SENSORS) {
            int sid = sqlite3_column_int(sensor_res, 0);
            const char *sname = (const char *)sqlite3_column_text(sensor_res, 1);
            init_sensor_stats(&sensor_stats[sensor_count], sid, sname ? sname : "", "");
            sensor_count++;
        }
        sqlite3_finalize(sensor_res);
    }

    // Collect all data and alerts
    AlertRecord alerts[MAX_ALERTS];
    int alert_count = 0;
    int total_alerts = 0;
    
    sqlite3_stmt *res = NULL;
    const char *sql =
        "SELECT r.id, r.sensor_id, s.name, r.model, r.measured_at, r.pm25, r.pm10, r.co, r.no2, r.o3, r.so2 "
        "FROM readings r JOIN sensors s ON s.id = r.sensor_id "
        "ORDER BY r.measured_at ASC;";

    if (sqlite3_prepare_v2(db, sql, -1, &res, NULL) == SQLITE_OK) {
        while (sqlite3_step(res) == SQLITE_ROW) {
            int sensor_id = sqlite3_column_int(res, 1);
            const char *sname = (const char *)sqlite3_column_text(res, 2);
            const char *model = (const char *)sqlite3_column_text(res, 3);
            const char *timestamp = (const char *)sqlite3_column_text(res, 4);
            double pm25 = sqlite3_column_double(res, 5);
            double pm10 = sqlite3_column_double(res, 6);
            double co = sqlite3_column_double(res, 7);
            double no2 = sqlite3_column_double(res, 8);
            double o3 = sqlite3_column_double(res, 9);
            double so2 = sqlite3_column_double(res, 10);
            
            // Find sensor index
            int sidx = -1;
            for (int i = 0; i < sensor_count; i++) {
                if (sensor_stats[i].sensor_id == sensor_id) {
                    sidx = i;
                    break;
                }
            }
            
            if (sidx >= 0) {
                if (sensor_stats[sidx].count == 0 && model) {
                    snprintf(sensor_stats[sidx].model, sizeof(sensor_stats[sidx].model), "%s", model);
                }
                update_sensor_stats(&sensor_stats[sidx], pm25, pm10, co, no2, o3, so2);
            }
            
            // Check for alerts
            if (pm25 > limit_pm25) {
                add_alert(alerts, &alert_count, sensor_id, sname, timestamp, "PM2.5", pm25, limit_pm25);
                total_alerts++;
            }
            if (pm10 > limit_pm10) {
                add_alert(alerts, &alert_count, sensor_id, sname, timestamp, "PM10", pm10, limit_pm10);
                total_alerts++;
            }
            if (co > limit_co) {
                add_alert(alerts, &alert_count, sensor_id, sname, timestamp, "CO", co, limit_co);
                total_alerts++;
            }
            if (no2 > limit_no2) {
                add_alert(alerts, &alert_count, sensor_id, sname, timestamp, "NO2", no2, limit_no2);
                total_alerts++;
            }
            if (o3 > limit_o3) {
                add_alert(alerts, &alert_count, sensor_id, sname, timestamp, "O3", o3, limit_o3);
                total_alerts++;
            }
            if (so2 > limit_so2) {
                add_alert(alerts, &alert_count, sensor_id, sname, timestamp, "SO2", so2, limit_so2);
                total_alerts++;
            }
        }
        sqlite3_finalize(res);
    }

    // Create PDF
    HPDF_Doc pdf = HPDF_New(NULL, NULL);
    if (!pdf) {
        fprintf(stderr, "Cannot create PDF document.\n");
        aqm_db_close(db);
        return;
    }

    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
    HPDF_Font font_bold = HPDF_GetFont(pdf, "Helvetica-Bold", NULL);

    int page_num = 0;

    // Cover page
    draw_cover_page(pdf, font_bold, font, sensor_count, total_records, total_alerts, period_start, period_end);
    page_num++;

    // Summary page
    draw_summary_page(pdf, font_bold, font, sensor_stats, sensor_count, &page_num);

    // Sensor detail pages
    for (int i = 0; i < sensor_count; i++) {
        draw_sensor_detail_page(pdf, font_bold, font, &sensor_stats[i], db, &page_num);
    }

    // Alerts page
    draw_alerts_page(pdf, font_bold, font, alerts, alert_count, &page_num);

    HPDF_SaveToFile(pdf, PDF_FILE);
    HPDF_Free(pdf);
    aqm_db_close(db);
    
    printf("PDF report generated: %s (%d pages)\n", PDF_FILE, page_num);
}

#else

void generate_pdf_report(void) {
    fputs("PDF export is disabled in this build (compile with HAVE_HPDF and link libharu).\n", stderr);
}

#endif
