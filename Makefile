# Air Quality Monitor — GCC/Clang (Linux, macOS, MSYS2/MinGW on Windows)

TARGET = air_quality_monitor

SRCS = main.c \
       globals.c \
       aqm_paths.c \
       aqm_platform.c \
       aqm_db.c \
       insert_data.c \
       fetch_data.c \
       alert_system.c \
       export_to_csv.c \
       configure_limits.c \
       generate_statistics.c \
       backup_database.c \
       data_cleanup.c \
       interval_collection.c \
       generate_pdf_report.c \
       config_persistence.c \
       dht22.c

CC ?= gcc
CFLAGS = -Wall -Wextra -std=c99

LIBS = -lsqlite3

# PDF via libharu: HAVE_HPDF=0 if libhpdf is not installed (typical on Windows unless you use vcpkg/MSYS).
ifeq ($(OS),Windows_NT)
  HAVE_HPDF ?= 0
else
  HAVE_HPDF ?= 1
endif

ifeq ($(HAVE_HPDF),1)
  CFLAGS += -DHAVE_HPDF
  LIBS += -lhpdf
endif

# Optional: wiringPi (Linux/Raspberry Pi). Keeps Windows/macOS builds working by default.
HAVE_WIRINGPI ?= 0
ifeq ($(HAVE_WIRINGPI),1)
  CFLAGS += -DHAVE_WIRINGPI
  LIBS += -lwiringPi
endif

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(TARGET) $(TARGET).exe *.o *.so
