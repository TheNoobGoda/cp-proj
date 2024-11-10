CC = mpicc

CFLAGS = -Wall -Wextra -O2

TARGET = fox

SRC = main.c

NUM_PROCS = 4

ARGS = matrix_examples/input6

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -lm

clean:
	rm -f $(TARGET)
	rm -f result*


run:
	mpirun --oversubscribe -np $(NUM_PROCS) fox $(ARGS)


# Phony targets
.PHONY: all clean run
