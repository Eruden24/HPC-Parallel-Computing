# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -fopenmp

# Source files
SRCS = main.c array.c

# Object files
OBJS = $(SRCS:.c=.o)

# Executable name
TARGET = main

# Default rule
all: $(TARGET)

# Link objects to create executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET)
