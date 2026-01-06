CC = gcc

# ------------------------
# Flags
# ------------------------

KERNEL_CFLAGS = -Wall -Wextra -Iinclude -g -D_GNU_SOURCE \
                -mno-red-zone

USER_CFLAGS = -Wall -Wextra -Iinclude -g \
              -ffreestanding -nostdlib -fno-builtin \
              -fno-stack-protector -mno-red-zone

LDFLAGS =

# ------------------------
# Directories
# ------------------------

SRC_DIR  = src
USER_DIR = src/user
OBJ_DIR  = build

TARGET = kernel

# ------------------------
# Source files
# ------------------------

KERNEL_SRCS = $(shell find $(SRC_DIR) -name "*.c" ! -path "$(USER_DIR)/*")
KERNEL_OBJS = $(KERNEL_SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

USER_SRCS = $(wildcard $(USER_DIR)/*.c)
USER_ELFS = $(USER_SRCS:%.c=%.elf)

# ------------------------
# Default target
# ------------------------

all: $(TARGET) $(USER_ELFS)

# ------------------------
# Kernel build
# ------------------------

$(TARGET): $(KERNEL_OBJS)
	$(CC) $(KERNEL_CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(KERNEL_CFLAGS) -c $< -o $@

# ------------------------
# User task build (freestanding)
# ------------------------

$(USER_DIR)/%.elf: $(USER_DIR)/%.c
	$(CC) $(USER_CFLAGS) -c $< -o $@.o
	$(CC) $(USER_CFLAGS) $@.o -o $@
	rm -f $@.o

# ------------------------
# Utilities
# ------------------------

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(USER_DIR)/*.elf $(USER_DIR)/*.o

run: all
	./$(TARGET)
