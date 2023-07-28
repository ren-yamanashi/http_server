.PHONY: build, prebuild

TARGET_DIR=target
CC=gcc

defalt:
	make build

build:
	make prebuild
	make $(TARGET_DIR)/libServer.dylib

prebuild:
	rm -rf $(TARGET_DIR)
	mkdir -p $(TARGET_DIR)

$(TARGET_DIR)/libServer.dylib: ./libServer.c
	$(CC) -shared -o $@ $^