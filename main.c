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

void send_matrix(int **matrix, int size, int submatrixsize, int numprocs){
    // int *sendbuf;
    // int pos = 0;
    //MPI_Pack(&matrix[0][index], size, MPI_INT, sendbuf, submatrixsize * submatrixsize, &pos, MPI_COMM_WORLD);
    //MPI_Pack(&matrix[1][index], size, MPI_INT, sendbuf, submatrixsize * submatrixsize, &pos, MPI_COMM_WORLD);
    //MPI_Pack(&matrix[2][index], size, MPI_INT, sendbuf, submatrixsize * submatrixsize, &pos, MPI_COMM_WORLD);

    //MPI_Bsend(sendbuf, pos, MPI_PACKED, 1, 0, MPI_COMM_WORLD);

    int col = 0;
    int row = 0;

    for (int i =0 ; i<numprocs; i++){
        MPI_Bsend(&matrix[col][row], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Bsend(&matrix[col][row+1], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Bsend(&matrix[col][row+2], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Bsend(&matrix[col+1][row], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Bsend(&matrix[col+1][row+1], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Bsend(&matrix[col+1][row+2], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Bsend(&matrix[col+2][row], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Bsend(&matrix[col+2][row+1], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Bsend(&matrix[col+2][row+2], 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        
        row += submatrixsize;
        if (row>=size){
            row = 0;
            col += submatrixsize;
        }
    }
}

void receive_matrix(int **matrix, int size){
    int recbuf;
    //int pos = 0;
    for (int i = 0; i<size; i++){
        for (int j =0; j<size; j++){
            MPI_Recv(&recbuf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            matrix[i][j] = recbuf;
        }

        // for (int j =0 ; j>size; j++){
        //     matrix[i][j] = recbuf[j];
        // }
    }

    
    // MPI_Recv(&recbuf, size*size, MPI_PACKED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    // int col;
    // MPI_Unpack(&recbuf, size*size, &pos, &col, size, MPI_INT, MPI_COMM_WORLD);
    // matrix[0] = &col;

    // int col1;
    // MPI_Unpack(&recbuf, size*size, &pos, &col1, size, MPI_INT, MPI_COMM_WORLD);
    // matrix[1] = &col1;

    // int col2;
    // MPI_Unpack(&recbuf, size*size, &pos, &col2, size, MPI_INT, MPI_COMM_WORLD);
    // matrix[2] = &col2;
    
    //free(recbuf);

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


    int **submatrix = malloc(submatrix_size * sizeof(int*));

    for (int i = 0; i<submatrix_size; i++){
        submatrix[i] = malloc(submatrix_size * sizeof(int));
    }

    if (rank == 0){
        print_matrix(matrix, size, size);
        send_matrix(matrix, size, 3, numprocs);
    }

    receive_matrix(submatrix,3);

    printf("rank: %d\n",rank);
    print_matrix(submatrix, submatrix_size, submatrix_size);

    freeMatrix(submatrix, submatrix_size);

    if (rank == 0){
        freeMatrix(matrix, size);
    }

    MPI_Finalize();
}
