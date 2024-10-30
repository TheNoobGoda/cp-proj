#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

int** read_graph(const char *file_name, int *size){
    int i,j;

    FILE *file = fopen(file_name, "r");

    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    fscanf(file, "%d", size);

    int **matrix = malloc(*size * sizeof(int*));

    for (i = 0; i<*size; i++){
        matrix[i] = malloc(*size * sizeof(int));
    }


    for (i = 0; i < *size; i++) {
        for (j = 0; j < *size; j++) {
            if (fscanf(file, "%d", &matrix[i][j]) != 1) {
                perror("Error reading matrix data");
                fclose(file);
                return NULL;
            }
        }
    }

    fclose(file);
    return matrix;
}

void print_matrix(int **matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

void freeMatrix(int **matrix, int size) {
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void matrix_mult(int **matrix, int size) {
    int **result = malloc(size * sizeof(int*));

    for (int i=0; i<size; i++){
        result[i] = malloc(size * sizeof(int));
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (i == j){
                result[i][j] = 0;
                continue;
            }
            if (matrix[i][j] == 0){
                result[i][j] = INT_MAX;
            }
            else{
                result[i][j] = matrix[i][j];
            }
            for (int k = 0; k < size; k++) {
                if (matrix[i][k] != 0 && matrix[k][j] != 0){
                    int path = matrix[i][k] + matrix[k][j];
                    if (path < result[i][j] ){
                        result[i][j] = path;
                    }
                }
            }
            if (result[i][j] == INT_MAX){
                result[i][j] = matrix[i][j];
            }
        }
    }

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = result[i][j];
        }
    }

    freeMatrix(result, size);
}

int save_result(const char *filename, int **matrix, int size){
    FILE *file = fopen(filename, "w");
    if (file == NULL){
        perror("Error opening file for writing");
        return 1;
    }

    for (int i =0; i<size; i++){
        for (int j=0; j<size; j++){
            if (j == size-1){
                fprintf(file,"%d", matrix[i][j]);    
            }
            else{fprintf(file,"%d ", matrix[i][j]);}
        }
        fprintf(file,"\n");
    }

    fclose(file);
    return 0;
}

int main(int argc, char *argv[]){
    int numprocs, rank, size;

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    // int **matrix = read_graph(filename, &size);
    // if (matrix==NULL){
    //     return 1;
    // }

    // int d =1;
    // while(d<size){
    //     matrix_mult(matrix, size);
    //     d *=2;
    // }

    // print_matrix(matrix, size, size);

    // save_result("result6", matrix, size);

    // freeMatrix(matrix, size);


    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int submatrix_size;
    int **matrix;

    if (rank == 0){

        matrix = read_graph(filename, &size);
        if (matrix==NULL){
            MPI_Finalize();
            return 1;
        }
        double q = sqrt(numprocs);
        double r = size % (int) q;

        if (q != floor(q) || r != 0){
         MPI_Finalize();
         printf("Wrong\n");
         return 1; 
        }

        submatrix_size = size / sqrt(numprocs);

    }

    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&submatrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);


    if (rank == 0){
        freeMatrix(matrix, size);
    }
    


    MPI_Finalize();
}
