CC = mpicc

CFLAGS = -Wall -Wextra -O2

TARGET = fox

SRC = main.c

NUM_PROCS = 1

ARGS = matrix_examples/input6

FILE1 = matrix_examples/input5

FILE2 = matrix_examples/input5

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

run:
	mpirun -np $(NUM_PROCS) fox $(ARGS)

compare_diff:
	diff $(FILE1) $(FILE2)

# Phony targets
.PHONY: all clean run
