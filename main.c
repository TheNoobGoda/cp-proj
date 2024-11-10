#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

/**
 * Reads a graph in the form of a adjacency matrix.
 * @param[in] file_name
 * @param[out] size
 * @return The matrix read from the file
 */ 

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

/**
 * Prints the input matrix to the console
 * @param[in] matrix
 * @param[in] rows
 * @param[in] cols
 */
void print_matrix(int **matrix, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }
}

/**
 * Frees the memory occupied by a matrix
 * @param[in] matrix
 * @param[in] size
 */
void freeMatrix(int **matrix, int size) {
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

/**
 * Min-plus multiplication of the matrix to calculate the shortest path between pairs of nodes of a graph in the form of a adjacency matrix
 * @param[in] matrix_a
 * @param[in] matrix_b
 * @param[in] result
 * @param[in] size
 * @param[out] result
 */
void min_plus_matrix_mult(int **matrix_a, int **matrix_b, int **result, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (result[i][j] == 0){
                result[i][j] = INT_MAX;
            }
            for (int k = 0; k < size; k++) {
                if (matrix_a[i][k] != 0 && matrix_b[k][j] != 0){
                    int path = matrix_a[i][k] + matrix_b[k][j];
                    if (path < result[i][j]){
                        result[i][j] = path;
                    }
                }
            }
            if (result[i][j] == INT_MAX){
                result[i][j] = 0;
            }
        }
    }
}

/**
 * Saves the matrix in a file
 * @param[in] filename
 * @param[in] matrix
 * @param[in] size
 * @returns 0 on successe and 1 if there was an error
 */
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

/**
 * Flattens and prepares a matrix to be scaterred trhough all the processes
 * @param[in] matrix
 * @param[out] flat_matrix
 * @param[in] size
 * @param[in] submatrixsize
 * @param[in] numprocs
 */
void flatten_main_matrix(int **matrix, int *flat_matrix, int size, int submatrixsize, int numprocs){
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

/**
 * Flattens a matrix in to a vector
 * @param[in] matrix
 * @param[out] flat_matrix
 * @param[in] size
 */
void flatten_matrix(int **matrix, int *flat_matrix, int size){
    for (int i=0; i<size; i++){
        for (int j=0; j<size; j++){
            flat_matrix[j+i*size] = matrix[i][j];
        }
    }
}

/**
 * Turns a vector representing a flat matrix in to a matrix and organizes the values from the gather of all the matrices
 * @param[out] matrix
 * @param[in] flat_matrix
 * @param[in] size
 * @param[in] submatrixsize
 * @param[in] numprocs
 */
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

/**
 * Turns a vector representing a flat matrix in to a matrix
 * @param[out] matrix
 * @param[in] flat_matrix
 * @param[in] size
 */
void unflatten_matrix(int **matrix, int *flat_matrix, int size){
    for (int i=0; i<size; i++){
        for (int j=0; j<size; j++){
            matrix[i][j] = flat_matrix[j+i*size];
        }
    }
}

/**
 * Copies a matrix
 * @param[in] original_matrix
 * @param[out] new_matrix
 * @param[in] size
 */
void copy_matrix(int **original_matrix, int **new_matrix, int size){
    for (int i =0; i<size; i++){
        for (int j=0; j<size; j++){
            new_matrix[i][j] = original_matrix[i][j];
        }
    }
}

int main(int argc, char *argv[]){
    // func init
    int numprocs, rank;

    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    
    int submatrix_size, size;
    int **matrix;
    double q;

    // matrix read
    if (rank == 0){
    
        matrix = read_graph(filename, &size);
        if (matrix==NULL){
            printf("Error reading matrix\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        q = sqrt(numprocs);
        if (q != floor(q)){
            MPI_Abort(MPI_COMM_WORLD, 2);
        }
        double r = size % (int) q;
    
        if (r != 0){
            MPI_Abort(MPI_COMM_WORLD, 3);
        }

        submatrix_size = size / sqrt(numprocs);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();
    
    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&submatrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&q, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    //matrices initialization
    int *flat_matrix = malloc(size*size*sizeof(int));
    

    int **submatrix = malloc(submatrix_size * sizeof(int*));

    for (int i = 0; i<submatrix_size; i++){
        submatrix[i] = malloc(submatrix_size * sizeof(int));
    }

    int *flat_submatrix = malloc(submatrix_size*submatrix_size*sizeof(int));


    int **submatrix_b = malloc(submatrix_size * sizeof(int*));

    for (int i = 0; i<submatrix_size; i++){
        submatrix_b[i] = malloc(submatrix_size * sizeof(int));
    }

    int *flat_submatrix_b = malloc(submatrix_size*submatrix_size * sizeof(int));


    int **aux_matrix = malloc(submatrix_size * sizeof(int*));

    for (int i = 0; i<submatrix_size; i++){
        aux_matrix[i] = malloc(submatrix_size * sizeof(int));
    }

    int *flat_aux_matrix = malloc(submatrix_size*submatrix_size * sizeof(int));


    int **result_submatrix = malloc(submatrix_size * sizeof(int*));

    for (int i = 0; i<submatrix_size; i++){
        result_submatrix[i] = malloc(submatrix_size * sizeof(int));
    }
    
    // distribute matrix    
    if (rank == 0){flatten_main_matrix(matrix, flat_matrix, size, submatrix_size, numprocs);}

    MPI_Scatter(flat_matrix, submatrix_size*submatrix_size, MPI_INT, flat_submatrix, submatrix_size*submatrix_size, MPI_INT, 0, MPI_COMM_WORLD);
    
    unflatten_matrix(submatrix, flat_submatrix, submatrix_size);

    copy_matrix(submatrix, result_submatrix, submatrix_size);

    //fox algorithm
    MPI_Comm row_comm;
    MPI_Comm_split(MPI_COMM_WORLD, (int)floor(rank / q), rank, &row_comm);
    int row_rank;
    MPI_Comm_rank(row_comm, &row_rank);

    MPI_Comm col_comm;
    MPI_Comm_split(MPI_COMM_WORLD, rank % (int)q, rank, &col_comm);
    int col_rank;
    MPI_Comm_rank(col_comm, &col_rank);

    copy_matrix(submatrix, submatrix_b, submatrix_size);

    int dest = col_rank-1;
    if (dest == -1){dest = q-1;}

    int source = col_rank+1;
    if (source == q){source = 0;}

    int broadcast_row_rank = col_rank;

    for (int j=0; j<size; j++){
        for (int i=0; i<q; i++){
            flatten_matrix(submatrix_b, flat_submatrix_b, submatrix_size);
            if (row_rank == broadcast_row_rank){
                flatten_matrix(submatrix, flat_aux_matrix, submatrix_size);
            }      
            MPI_Bcast(flat_aux_matrix, submatrix_size*submatrix_size, MPI_INT, broadcast_row_rank, row_comm);
            unflatten_matrix(aux_matrix, flat_aux_matrix, submatrix_size);
        
            min_plus_matrix_mult(aux_matrix, submatrix_b, result_submatrix, submatrix_size);

            MPI_Sendrecv(flat_submatrix_b, submatrix_size*submatrix_size, MPI_INT, dest, 0, flat_aux_matrix, submatrix_size*submatrix_size, MPI_INT, source, 0, col_comm, MPI_STATUS_IGNORE);
            unflatten_matrix(submatrix_b, flat_aux_matrix, submatrix_size);
            broadcast_row_rank ++;
            if (broadcast_row_rank >= q){broadcast_row_rank = 0;}
        }
        copy_matrix(result_submatrix, submatrix, submatrix_size);
        copy_matrix(result_submatrix, submatrix_b, submatrix_size);
    }

    
    MPI_Comm_free(&row_comm);
    MPI_Comm_free(&col_comm);

    // gather matrix
    flatten_matrix(result_submatrix, flat_submatrix, submatrix_size);
    MPI_Gather(flat_submatrix, submatrix_size*submatrix_size, MPI_INT, flat_matrix, submatrix_size*submatrix_size, MPI_INT, 0, MPI_COMM_WORLD);

    // free memory
    freeMatrix(submatrix_b, submatrix_size);
    freeMatrix(aux_matrix, submatrix_size);
    freeMatrix(submatrix, submatrix_size);
    freeMatrix(result_submatrix, submatrix_size);
    free(flat_submatrix);
    free(flat_aux_matrix);

    if (rank == 0){
        unflatten_main_matrix(matrix, flat_matrix, size, submatrix_size, numprocs);
        for (int i=0; i<size;i++){
            matrix[i][i] = 0;
        }
        //print_matrix(matrix, size, size);
        save_result("result", matrix, size);
        freeMatrix(matrix, size);
        free(flat_matrix);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();
    if (rank ==0) {printf("Execution time: %f\n",end-start);}
    
    MPI_Finalize();
}
