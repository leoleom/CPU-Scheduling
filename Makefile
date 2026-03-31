CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -Iinclude

SRC_DIR = src

# sources and objects
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/scheduler.c \
       $(SRC_DIR)/process.c \
       $(SRC_DIR)/heap.c \
       $(SRC_DIR)/fcfs.c \
       $(SRC_DIR)/sjf.c \
       $(SRC_DIR)/stcf.c \
       $(SRC_DIR)/rr.c \
       $(SRC_DIR)/mlfq.c \
       $(SRC_DIR)/metrics.c \
       $(SRC_DIR)/gantt.c \
	$(SRC_DIR)/utils.c \
       $(SRC_DIR)/compare.c

OBJS = $(SRCS:.c=.o)                
TARGET = schedsim

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

test: all
	@echo "Running automated test suite..."
	@chmod +x tests/test_suite.sh
	@./tests/test_suite.sh

.PHONY: all clean