CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Iinclude -g

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

TARGET = $(BUILD_DIR)/schedsim 

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	@echo "Build successful! Run with: ./$(TARGET) --algorithm=FCFS --processes=\"A:0:240\""

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean