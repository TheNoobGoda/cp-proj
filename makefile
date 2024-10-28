CC = mpicc

CFLAGS = -Wall -Wextra -O2

TARGET = fox

SRC = main.c

NUM_PROCS = 4

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run:
	mpirun -np $(NUM_PROCS) fox

# Phony targets
.PHONY: all clean
