CC = gcc
RC = windres
CFLAGS = -Wall -Wextra -O2 -std=c99 -I.
LIBS = -luser32 -lkernel32 -lshell32 -lgdi32 -lcomctl32 -lcomdlg32
LDFLAGS = -mwindows
TARGET = mouse_stabilizer.exe
SOURCES = main.c mouse_input.c smooth_engine.c target_pointer.c hotkey.c tray_ui.c config.c settings_ui.c
OBJECTS = $(SOURCES:.c=.o)
RESOURCE_RC = mouse_stabilizer.rc
RESOURCE_OBJ = mouse_stabilizer_res.o

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS) $(RESOURCE_OBJ)
	$(CC) $(OBJECTS) $(RESOURCE_OBJ) -o $(TARGET) $(LDFLAGS) $(LIBS)

$(RESOURCE_OBJ): $(RESOURCE_RC)
	$(RC) -I. --input-format=rc --output-format=coff $(RESOURCE_RC) $(RESOURCE_OBJ)

%.o: %.c mouse_stabilizer.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(RESOURCE_OBJ) $(TARGET) *.log *.ini

install: $(TARGET)
	copy $(TARGET) "C:\Program Files\MouseStabilizer\"
	
debug: CFLAGS += -DDEBUG -g
debug: LDFLAGS = -mconsole
debug: $(TARGET)

release: CFLAGS += -DNDEBUG
release: $(TARGET)
	strip $(TARGET)

help:
	@echo "Available targets:"
	@echo "  all     - Build the mouse stabilizer (default, no console window)"
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Build with debug information and console window"
	@echo "  release - Build optimized release version (no console window)"
	@echo "  install - Install to Program Files"
	@echo "  help    - Show this help message"