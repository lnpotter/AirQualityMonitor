#include "core/aqm_db.h"
#include <stdio.h>
#include <sqlite3.h>

#ifdef HAVE_HPDF
#include <hpdf.h>

#define PDF_FILE "sensor_data_report.pdf"

void generate_pdf_report(void) {
    sqlite3 *db = NULL;
    if (aqm_db_open(&db) != 0)
        return;

    sqlite3_stmt *res = NULL;
    const char *sql =
        "SELECT r.id, r.sensor_id, s.name, r.model, r.measured_at, r.pm25, r.pm10, r.co, r.no2, r.o3, r.so2 "
        "FROM readings r JOIN sensors s ON s.id = r.sensor_id "
        "ORDER BY r.measured_at ASC LIMIT 500;";

    if (sqlite3_prepare_v2(db, sql, -1, &res, NULL) != SQLITE_OK) {
        fprintf(stderr, "Query failed: %s\n", sqlite3_errmsg(db));
        aqm_db_close(db);
        return;
    }

    HPDF_Doc pdf = HPDF_New(NULL, NULL);
    if (!pdf) {
        fprintf(stderr, "Cannot create PDF document.\n");
        sqlite3_finalize(res);
        aqm_db_close(db);
        return;
    }

    HPDF_Page page = HPDF_AddPage(pdf);
    HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
    HPDF_Font font = HPDF_GetFont(pdf, "Helvetica", NULL);
    HPDF_Page_SetFontAndSize(page, font, 9);

    HPDF_REAL y = HPDF_Page_GetHeight(page) - 40;
    const HPDF_REAL line = 12;
    const HPDF_REAL x0 = 40;

    HPDF_Page_BeginText(page);
    HPDF_Page_TextOut(page, x0, y, "Air quality readings (max 500 rows)");
    y -= line * 2;

    HPDF_Page_TextOut(page, x0, y, "id");
    HPDF_Page_TextOut(page, x0 + 40, y, "sid");
    HPDF_Page_TextOut(page, x0 + 80, y, "sensor");
    HPDF_Page_TextOut(page, x0 + 170, y, "model");
    HPDF_Page_TextOut(page, x0 + 250, y, "time");
    HPDF_Page_TextOut(page, x0 + 390, y, "pm25/pm10/co...");
    y -= line;

    while (sqlite3_step(res) == SQLITE_ROW) {
        char linebuf[256];
        snprintf(linebuf, sizeof(linebuf), "%d", sqlite3_column_int(res, 0));
        HPDF_Page_TextOut(page, x0, y, linebuf);
        snprintf(linebuf, sizeof(linebuf), "%d", sqlite3_column_int(res, 1));
        HPDF_Page_TextOut(page, x0 + 40, y, linebuf);
        snprintf(linebuf, sizeof(linebuf), "%s", sqlite3_column_text(res, 2));
        HPDF_Page_TextOut(page, x0 + 80, y, linebuf);
        snprintf(linebuf, sizeof(linebuf), "%s", sqlite3_column_text(res, 3));
        HPDF_Page_TextOut(page, x0 + 170, y, linebuf);
        snprintf(linebuf, sizeof(linebuf), "%s", sqlite3_column_text(res, 4));
        HPDF_Page_TextOut(page, x0 + 250, y, linebuf);
        snprintf(linebuf, sizeof(linebuf), "%.1f/%.1f/%.1f", sqlite3_column_double(res, 5),
                 sqlite3_column_double(res, 6), sqlite3_column_double(res, 7));
        HPDF_Page_TextOut(page, x0 + 390, y, linebuf);

        y -= line;
        if (y < 50) {
            HPDF_Page_EndText(page);
            page = HPDF_AddPage(pdf);
            HPDF_Page_SetSize(page, HPDF_PAGE_SIZE_A4, HPDF_PAGE_PORTRAIT);
            HPDF_Page_SetFontAndSize(page, font, 9);
            HPDF_Page_BeginText(page);
            y = HPDF_Page_GetHeight(page) - 40;
        }
    }

    HPDF_Page_EndText(page);
    sqlite3_finalize(res);
    aqm_db_close(db);

    HPDF_SaveToFile(pdf, PDF_FILE);
    HPDF_Free(pdf);
    printf("PDF report generated: %s\n", PDF_FILE);
}

#else

void generate_pdf_report(void) {
    fputs("PDF export is disabled in this build (compile with HAVE_HPDF and link libharu).\n", stderr);
}

#endif
