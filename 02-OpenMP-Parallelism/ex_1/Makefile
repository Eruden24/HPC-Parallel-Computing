CC = gcc

# Compilation flags
CFLAGS = -Wall -Wextra -O3 -pthread -fopenmp

# Linker flags
LDFLAGS = -lm -fopenmp

TARGET = polymul
SRCS = main.c polynomial.c
OBJDIR = obj

OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(SRCS))

# Default target
all: $(TARGET)

# Ensure object directory exists
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Link the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Compile source files into obj/
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build
clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
