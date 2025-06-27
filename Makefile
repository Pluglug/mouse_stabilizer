CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99 -I.
LIBS = -luser32 -lkernel32 -lshell32 -lgdi32 -lcomctl32
TARGET = mouse_stabilizer.exe
SOURCES = main.c mouse_input.c smooth_engine.c target_pointer.c hotkey.c tray_ui.c config.c settings_ui.c
OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)

%.o: %.c mouse_stabilizer.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) *.log *.ini

install: $(TARGET)
	copy $(TARGET) "C:\Program Files\MouseStabilizer\"
	
debug: CFLAGS += -DDEBUG -g
debug: $(TARGET)

release: CFLAGS += -DNDEBUG
release: $(TARGET)
	strip $(TARGET)

help:
	@echo "Available targets:"
	@echo "  all     - Build the mouse stabilizer (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  debug   - Build with debug information"
	@echo "  release - Build optimized release version"
	@echo "  install - Install to Program Files"
	@echo "  help    - Show this help message"