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

void flatten_matrix(int **matrix, int *flat_matrix, int size){
    for (int i=0; i<size; i++){
        for (int j=0; j<size; j++){
            flat_matrix[j+i*size] = matrix[i][j];
        }
    }
}

void send_matrix(int *flat_matrix, int size, int submatrixsize, int numprocs, MPI_Datatype matrixType){
    int col = 0;
    int row = 0;

    for (int i =0 ; i<numprocs; i++){
        MPI_Bsend(&flat_matrix[col+row*size], 1, matrixType, i, 0, MPI_COMM_WORLD);
        
        col += submatrixsize;
        if (col>=size){
            col = 0;
            row += submatrixsize;
        }
    } 
}

void receive_matrix(int **matrix, int size, MPI_Datatype matrixType){
    int *recbuf = malloc(size*size*sizeof(int));

    MPI_Recv(recbuf, 1, matrixType, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("buffer: ");

    for (int i =0;i<size*size; i++){
        printf("%d ",recbuf[i]);
    }

    printf("\n");

    for (int i =0; i<size; i++){
        for (int j =0; j<size; j++){
            matrix[i][j] = recbuf[j+i*size];
        }
    }
}

int main(int argc, char *argv[]){
    int numprocs, rank, size;

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int submatrix_size;
    int **matrix;
    int *flat_matrix;

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
        flat_matrix = malloc(size*size*sizeof(int));
        flatten_matrix(matrix, flat_matrix, size);

    }

    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&submatrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);


    int **submatrix = malloc(submatrix_size * sizeof(int*));

    for (int i = 0; i<submatrix_size; i++){
        submatrix[i] = malloc(submatrix_size * sizeof(int));
    }

    MPI_Datatype matrixType;

    printf("matrixtype: %d %d\n",submatrix_size, size);

    MPI_Type_vector(submatrix_size, submatrix_size, size,  MPI_INT, &matrixType);
    MPI_Type_commit(&matrixType);


    if (rank == 0){
        print_matrix(matrix, size, size);
        send_matrix(flat_matrix, size, submatrix_size, numprocs, matrixType);
    }

    receive_matrix(submatrix,submatrix_size, matrixType);

    printf("rank: %d\n",rank);
    print_matrix(submatrix, submatrix_size, submatrix_size);

    freeMatrix(submatrix, submatrix_size);

    if (rank == 0){
        freeMatrix(matrix, size);
        free(flat_matrix);
    }

    MPI_Type_free(&matrixType);

    MPI_Finalize();
}
