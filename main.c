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

void flatten_matrix(int **matrix, int *flat_matrix, int size, int submatrixsize, int numprocs){
    int row = 0;
    int col = 0;
    int pos = 0;
    for (int i =0; i<numprocs; i++){
        for (int j = 0; j<submatrixsize; j++){
            for (int k = 0; k<submatrixsize; k++){
                flat_matrix[pos] = matrix[row+j][col+k];
                pos ++;
            }
        }
        col += submatrixsize;
        if (col>=size){
            col = 0;
            row += submatrixsize;
        }
    }
}

void unflatten_main_matrix(int **matrix, int *flat_matrix, int size, int submatrixsize, int numprocs){
    int row = 0;
    int col = 0;
    int pos = 0;

    for (int i=0; i<numprocs; i++){
        for (int j=0; j<submatrixsize; j++){
            for (int k=0; k<submatrixsize; k++){
                matrix[row+j][col+k] = flat_matrix[pos];
                pos ++;
            }
        }
        col += submatrixsize;
        if (col>=size){
            col = 0;
            row += submatrixsize;
        }
    }
    
}

void unflatten_matrix(int **matrix, int *flat_matrix, int size){
    for (int i=0; i<size; i++){
        for (int j=0; j<size; j++){
            matrix[i][j] = flat_matrix[j+i*size];
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
    // func init
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
    double q = sqrt(numprocs);
    double r = size % (int) q;

    // matrix read
    if (rank == 0){

        matrix = read_graph(filename, &size);
        if (matrix==NULL){
            MPI_Finalize();
            return 1;
        }

        if (q != floor(q) || r != 0){
         MPI_Finalize();
         printf("Wrong\n");
         return 1; 
        }

        submatrix_size = size / sqrt(numprocs);
        
    }

    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&submatrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //matrices initialization
    int **submatrix = malloc(submatrix_size * sizeof(int*));

    for (int i = 0; i<submatrix_size; i++){
        submatrix[i] = malloc(submatrix_size * sizeof(int));
    }

    int **result_submatrix = malloc(submatrix_size * sizeof(int*));

    for (int i = 0; i<submatrix_size; i++){
        result_submatrix[i] = malloc(submatrix_size * sizeof(int));
    }

    int *flat_matrix = malloc(size*size*sizeof(int));

    int *flat_submatrix = malloc(submatrix_size*submatrix_size*sizeof(int));

    // distribute matrix    
    if (rank == 0){flatten_matrix(matrix, flat_matrix, size, submatrix_size, numprocs);}

    MPI_Scatter(flat_matrix, submatrix_size*submatrix_size, MPI_INT, flat_submatrix, submatrix_size*submatrix_size, MPI_INT, 0, MPI_COMM_WORLD);
    
    unflatten_matrix(submatrix, flat_submatrix, submatrix_size);

    //fox algorithm
    MPI_Group world_group;
    MPI_Group *group_vector = malloc(q * sizeof(MPI_Group));
    MPI_Comm *comm_vector = malloc(q * sizeof(MPI_Comm));

    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    for (int i=0; i<q; i++){
        int *ranks = malloc(q * sizeof(int));
        for (int j=0; j<q; j++){
            ranks[j] = q*i+j;
        }

        MPI_Group_incl(world_group, q, ranks, &group_vector[i]);
        MPI_Comm_create(MPI_COMM_WORLD, group_vector[i], &comm_vector[i]);
        MPI_Group_free(&group_vector[i]);
        free(ranks);
    }
    MPI_Group_free(&world_group);

    // gather matrix
    MPI_Gather(flat_submatrix, submatrix_size*submatrix_size, MPI_INT, flat_matrix, submatrix_size*submatrix_size, MPI_INT, 0, MPI_COMM_WORLD);

    // free memory
    freeMatrix(submatrix, submatrix_size);
    freeMatrix(result_submatrix, submatrix_size);
    free(flat_submatrix);

    if (rank == 0){
        unflatten_main_matrix(matrix, flat_matrix, size, submatrix_size, numprocs);
        print_matrix(matrix, size, size);
        freeMatrix(matrix, size);
        free(flat_matrix);
    }

    for (int i=0; i<q; i++){
        if (comm_vector[i] != MPI_COMM_NULL){
            MPI_Comm_free(&comm_vector[i]);
        }
        
    }

    MPI_Finalize();
}
