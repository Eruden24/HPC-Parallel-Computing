CC = gcc
CFLAGS = -Wall -O2 -pthread
TARGET = bank_sim
SRC = main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)