TARGET = main

# Compiler
CC = gcc

# Flags
CFLAGS = -Wall -Wextra -O2 -std=c11 -pthread

SRCS = main.c

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET) *.o

.PHONY: clean
