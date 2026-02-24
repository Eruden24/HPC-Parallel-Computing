# Compiler
CC = gcc
CFLAGS = -Wall -fopenmp

# Directories
OBJDIR = obj
SRCS = main.c csr.c sparse.c
OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

# Target executable
TARGET = main

# Default target
all: $(OBJDIR) $(TARGET)

# Create object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Build executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Build object files
$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
