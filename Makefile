CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Iinclude -g -MMD -MP

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
TEST_DIR = tests

TARGET = $(BUILD_DIR)/schedsim 

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
DEPS = $(OBJS:.o=.d)

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

test: all
	@echo "Running automated test suite..."
	@cd $(TEST_DIR) && bash test_all_algorithms.sh

-include $(DEPS)

.PHONY: all clean test