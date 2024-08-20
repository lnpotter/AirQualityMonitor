#include <hpdf.h>
#include <stdio.h>
#include <sqlite3.h>

#define DB_NAME "air_quality.db"
#define PDF_FILE "sensor_data_report.pdf"

void generate_pdf_report() {
    sqlite3 *db;
    sqlite3_stmt *res;

    HPDF_Doc pdf;
    HPDF_Page page;
    HPDF_Font font;
    HPDF_REAL page_height = 800;
    HPDF_REAL y = page_height - 50;
    HPDF_REAL line_height = 20;

    pdf = HPDF_New(NULL, NULL);
    if (!pdf) {
        fprintf(stderr, "Error: Cannot create PDF document.\n");
        return;
    }

    page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);

    font = HPDF_GetFont(pdf, "Helvetica", NULL);
    HPDF_Page_SetFontAndSize(page, font, 12);

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, 50, y, "Sensor Data Report");

    y -= line_height * 2;
    HPDF_Page_TextOut(page, 50, y, "Sensor Name");
    HPDF_Page_TextOut(page, 200, y, "Data");
    HPDF_Page_TextOut(page, 300, y, "Timestamp");

    y -= line_height;

    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        HPDF_Free(pdf);
        return;
    }

    char *sql = "SELECT * FROM SensorData;";
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to execute query: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        HPDF_Free(pdf);
        return;
    }

    while (sqlite3_step(res) == SQLITE_ROW) {
        const char *sensor_name = (const char *)sqlite3_column_text(res, 1);
        float data = sqlite3_column_double(res, 2);
        const char *timestamp = (const char *)sqlite3_column_text(res, 3);

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s", sensor_name);
        HPDF_Page_TextOut(page, 50, y, buffer);

        snprintf(buffer, sizeof(buffer), "%.2f", data);
        HPDF_Page_TextOut(page, 200, y, buffer);

        snprintf(buffer, sizeof(buffer), "%s", timestamp);
        HPDF_Page_TextOut(page, 300, y, buffer);

        y -= line_height;
        if (y < 50) {
            HPDF_Page_EndText(page);
            page = HPDF_AddPage(pdf);
            HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
            HPDF_Page_SetFontAndSize(page, font, 12);
            HPDF_Page_BeginText(page);
            y = page_height - 50;
        }
    }

    HPDF_Page_EndText(page);
    sqlite3_finalize(res);
    sqlite3_close(db);

    HPDF_SaveToFile(pdf, PDF_FILE);
    HPDF_Free(pdf);

    printf("PDF report generated successfully: %s\n", PDF_FILE);
}
