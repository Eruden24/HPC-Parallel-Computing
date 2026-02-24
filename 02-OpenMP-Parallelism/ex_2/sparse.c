#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "sparse.h"

/* --- Initialize sparse matrix --- */
int **sparse_matrix_initialize(int N, double sparsity) {
    int total = N * N;
    int K = (int)(total * (1.0 - sparsity));

    unsigned int seed = (unsigned int)time(NULL);

    int **matrix = malloc(N * sizeof(int*));
    if (!matrix) exit(EXIT_FAILURE);

    for (int i = 0; i < N; i++) {
        matrix[i] = calloc(N, sizeof(int));
        if (!matrix[i]) exit(EXIT_FAILURE);
    }

    int (*coords)[2] = malloc(total * sizeof *coords);
    for (int idx=0, i=0; i<N; i++)
        for (int j=0; j<N; j++, idx++) {
            coords[idx][0] = i;
            coords[idx][1] = j;
        }

    // Shuffle coordinates
    for (int i=total-1; i>0; i--) {
        int r = rand_r(&seed) % (i+1);
        int t0 = coords[i][0], t1 = coords[i][1];
        coords[i][0] = coords[r][0];
        coords[i][1] = coords[r][1];
        coords[r][0] = t0;
        coords[r][1] = t1;
    }

    for (int x=0; x<K; x++) {
        int i = coords[x][0];
        int j = coords[x][1];
        matrix[i][j] = rand_r(&seed)%10 + 1; 
    }

    free(coords);
    return matrix;
}

void sparse_matrix_free(int **matrix, int N) {
    for (int i = 0; i < N; i++)
        free(matrix[i]);
    free(matrix);
}

void sparse_matrix_print(int **matrix, int N) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            printf("%d ", matrix[i][j]);
        printf("\n");
    }
}

/* --- Serial dense mat-vec --- */
int *sparse_matrix_multiply_vector(int **matrix, int N, int *vector) {
    int *result = (int*)calloc(N, sizeof(int));
    if (!result) exit(EXIT_FAILURE);

    for (int i=0; i<N; i++)
        for (int j=0; j<N; j++)
            result[i] += matrix[i][j]*vector[j];

    return result;
}

/* --- Parallel dense mat-vec --- */
int* sparse_matrix_multiply_vector_parallel(int **matrix, int N, int *vector) {
    int *result = (int*)calloc(N, sizeof(int));
    if (!result) exit(EXIT_FAILURE);

    #pragma omp parallel for schedule(static)
    for (int i=0; i<N; i++) {
        int sum = 0;
        for (int j=0; j<N; j++)
            sum += matrix[i][j]*vector[j];
        result[i] = sum;
    }

    return result;
}
