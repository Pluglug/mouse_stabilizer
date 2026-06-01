APP_NAME = MouseStabilizerMac
BUILD_DIR = build
APP_DIR = $(BUILD_DIR)/$(APP_NAME).app
MACOS_DIR = $(APP_DIR)/Contents/MacOS
RESOURCES_DIR = $(APP_DIR)/Contents/Resources
SOURCES = MouseStabilizerMac.swift

.PHONY: all clean run

all: $(APP_DIR)

$(APP_DIR): $(SOURCES) Info.plist
	rm -rf $(APP_DIR)
	mkdir -p $(MACOS_DIR) $(RESOURCES_DIR)
	cp Info.plist $(APP_DIR)/Contents/Info.plist
	swiftc -O -parse-as-library -framework Cocoa -framework ApplicationServices $(SOURCES) -o $(MACOS_DIR)/$(APP_NAME)

run: $(APP_DIR)
	open $(APP_DIR)

clean:
	rm -rf $(BUILD_DIR)
