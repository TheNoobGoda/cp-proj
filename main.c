#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int** read_graph(){
    int** matrix = malloc(4 * sizeof(int*));

    for (int i = 0; i<4; i++){
        matrix[i] = malloc(4 * sizeof(int));
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            matrix[i][j] = i + j;  // Example initialization
        }
    }

    return matrix;
}

void print_matrix(int ** matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);  // Print each element
        }
        printf("\n");  // Newline at the end of each row
    }
}

int main(int argc, char *argv[]){
    int numprocs, rank;

    int** matrix = read_graph();
    print_matrix(matrix, 4, 4);


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // printf("Hello world from proc %d\n",rank);

    MPI_Finalize();
}