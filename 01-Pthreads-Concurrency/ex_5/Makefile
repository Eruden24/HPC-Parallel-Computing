CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread -Iinclude
SRC = src/main.c src/cond_barrier.c src/sense_barrier.c src/pthread_barrier_wrapper.c
OBJDIR = build
BINDIR = bin

THREADS = 2 4 8 16
LOOPS = 10 50 100 500 1000

# Default target: build all
all: pthread cond sense

# Create directories
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(BINDIR):
	mkdir -p $(BINDIR)

# Compile pthread version
pthread: $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -DPTHREAD_BARRIER -o $(BINDIR)/pthread $(SRC)

# Compile cond barrier version
cond: $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -DCOND_BARRIER -o $(BINDIR)/cond $(SRC)

# Compile sense barrier version
sense: $(OBJDIR) $(BINDIR)
	$(CC) $(CFLAGS) -DSENSE_BARRIER -o $(BINDIR)/sense $(SRC)

# Clean build and binaries
clean:
	rm -rf $(OBJDIR) $(BINDIR)
