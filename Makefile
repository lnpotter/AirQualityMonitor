# Air Quality Monitor — GCC/Clang (Linux, macOS, MSYS2/MinGW on Windows)

TARGET = air_quality_monitor

SRC_DIR = src
INC_DIR = include
PLUGIN_DIR = plugins

SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/globals.c \
       $(SRC_DIR)/aqm_paths.c \
       $(SRC_DIR)/aqm_platform.c \
       $(SRC_DIR)/aqm_db.c \
       $(SRC_DIR)/sensor_loader.c \
       $(SRC_DIR)/sensor_info.c \
       $(SRC_DIR)/insert_data.c \
       $(SRC_DIR)/fetch_data.c \
       $(SRC_DIR)/alert_system.c \
       $(SRC_DIR)/export_to_csv.c \
       $(SRC_DIR)/configure_limits.c \
       $(SRC_DIR)/generate_statistics.c \
       $(SRC_DIR)/backup_database.c \
       $(SRC_DIR)/data_cleanup.c \
       $(SRC_DIR)/interval_collection.c \
       $(SRC_DIR)/generate_pdf_report.c \
       $(SRC_DIR)/config_persistence.c

CC ?= gcc
CFLAGS = -Wall -Wextra -std=c99 -I$(INC_DIR)

LIBS = -lsqlite3
SHARED_CFLAGS = -Wall -Wextra -std=c99 -I$(INC_DIR)

ifeq ($(OS),Windows_NT)
  PLUGIN_EXT = dll
  SHARED_FLAGS = -shared
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Darwin)
    PLUGIN_EXT = dylib
    SHARED_FLAGS = -dynamiclib
  else
    PLUGIN_EXT = so
    SHARED_FLAGS = -shared -fPIC
    LIBS += -ldl
  endif
endif

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

.PHONY: all clean plugins

all: $(TARGET)

plugins: $(PLUGIN_DIR)/dht22_plugin.$(PLUGIN_EXT) $(PLUGIN_DIR)/bme680_plugin.$(PLUGIN_EXT) $(PLUGIN_DIR)/pms5003_plugin.$(PLUGIN_EXT) $(PLUGIN_DIR)/mhz19_plugin.$(PLUGIN_EXT)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(PLUGIN_DIR)/dht22_plugin.$(PLUGIN_EXT): $(PLUGIN_DIR)/dht22_plugin.c $(INC_DIR)/sensor.h $(INC_DIR)/globals.h
	$(CC) $(SHARED_CFLAGS) $(SHARED_FLAGS) -o $@ $<

$(PLUGIN_DIR)/bme680_plugin.$(PLUGIN_EXT): $(PLUGIN_DIR)/bme680_plugin.c $(INC_DIR)/sensor.h $(INC_DIR)/globals.h
	$(CC) $(SHARED_CFLAGS) $(SHARED_FLAGS) -o $@ $<

$(PLUGIN_DIR)/pms5003_plugin.$(PLUGIN_EXT): $(PLUGIN_DIR)/pms5003_plugin.c $(INC_DIR)/sensor.h $(INC_DIR)/globals.h
	$(CC) $(SHARED_CFLAGS) $(SHARED_FLAGS) -o $@ $<

$(PLUGIN_DIR)/mhz19_plugin.$(PLUGIN_EXT): $(PLUGIN_DIR)/mhz19_plugin.c $(INC_DIR)/sensor.h $(INC_DIR)/globals.h
	$(CC) $(SHARED_CFLAGS) $(SHARED_FLAGS) -o $@ $<

clean:
	rm -f $(TARGET) $(TARGET).exe *.o *.so *.dylib *.dll plugins/*.so plugins/*.dylib plugins/*.dll
