# Name of the final executable
TARGET = air_quality_monitor

# List of all source files
SRCS = main.c \
       globals.c \
       create_database.c \
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
       config_persistence.c

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lsqlite3 -lncurses -lhpdf

# Default rule
all: $(TARGET)

# Compile all files and generate the final binary
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Clean compiled files
clean:
	rm -f $(TARGET) *.o

# Full clean, including backups and PDFs
clean_all: clean
	rm -f backup_*.db *.pdf config.cfg
