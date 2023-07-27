.PHONY: build, prebuild

TARGET_DIR=target
CC=gcc

defalt:
	make build

build:
	make prebuild
	make $(TARGET_DIR)/binding.dylib

prebuild:
	rm -rf $(TARGET_DIR)
	mkdir -p $(TARGET_DIR)

$(TARGET_DIR)/binding.dylib: ./binding.c
	$(CC) -shared -o $@ $^