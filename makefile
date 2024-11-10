CC = mpicc

CFLAGS = -Wall -Wextra -O2

TARGET = fox

SRC = main.c

NUM_PROCS = 4

ARGS = matrix_examples/input6

FILE1 = matrix_examples/output6

FILE2 = result6

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -lm

clean:
	rm -f $(TARGET)
	rm -f result*


run:
	mpirun --oversubscribe -np $(NUM_PROCS) fox $(ARGS)


run_file:
	mpirun --oversubscribe -np $(NUM_PROCS) fox $(ARGS) -hostfile $(HOSTFILE)
compare_diff:
	diff $(FILE1) $(FILE2)

# Phony targets
.PHONY: all clean run
