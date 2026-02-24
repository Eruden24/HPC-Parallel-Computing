# Makefile for main program

CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
TARGET = main
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o
