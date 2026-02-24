#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "utils.h"

int* generate_dense_matrix(int N, double sparsity) {
    long total_elements = (long)N * N;
    int *matrix = (int*)calloc(total_elements, sizeof(int));
    if (!matrix) { fprintf(stderr, "Alloc failed\n"); exit(1); }

    int nnz_target = (int)(total_elements * (1.0 - sparsity));
    
    // Simple random fill (could be optimized for large N but fits exercise scope)
    // Using a deterministic way or shuffling is better, but simple rand is fine here
    // provided N isn't massive.
    
    /* Efficient random filling for sparse matrices */
    for (int k = 0; k < nnz_target; k++) {
        int r = rand() % N;
        int c = rand() % N;
        while (matrix[r * N + c] != 0) { // Collision handling
             r = rand() % N;
             c = rand() % N;
        }
        matrix[r * N + c] = (rand() % 10) + 1;
    }
    return matrix;
}

int* generate_vector(int N) {
    int *vec = (int*)malloc(N * sizeof(int));
    for (int i = 0; i < N; i++) vec[i] = (rand() % 10) + 1;
    return vec;
}

CSR* dense_to_csr(int *matrix, int N) {
    CSR *csr = (CSR*)malloc(sizeof(CSR));
    csr->num_rows = N;
    
    // Count NNZ
    int nnz = 0;
    long total = (long)N * N;
    for (long i = 0; i < total; i++) {
        if (matrix[i] != 0) nnz++;
    }
    csr->nnz = nnz;

    csr->values = (int*)malloc(nnz * sizeof(int));
    csr->col_ind = (int*)malloc(nnz * sizeof(int));
    csr->row_ptr = (int*)malloc((N + 1) * sizeof(int));

    int count = 0;
    int row_idx = 0;
    csr->row_ptr[0] = 0;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int val = matrix[i * N + j];
            if (val != 0) {
                csr->values[count] = val;
                csr->col_ind[count] = j;
                count++;
            }
        }
        csr->row_ptr[i + 1] = count;
    }
    return csr;
}

void free_csr(CSR *csr) {
    if (csr) {
        if (csr->values) free(csr->values);
        if (csr->col_ind) free(csr->col_ind);
        if (csr->row_ptr) free(csr->row_ptr);
        free(csr);
    }
}

/* Local Matrix-Vector Multiplication for CSR */
void csr_mat_vec_mult(CSR *local_csr, int *x, int *local_y, int local_N) {
    for (int i = 0; i < local_N; i++) {
        int sum = 0;
        int start = local_csr->row_ptr[i];
        int end = local_csr->row_ptr[i+1];
        
        for (int j = start; j < end; j++) {
            sum += local_csr->values[j] * x[local_csr->col_ind[j]];
        }
        local_y[i] = sum;
    }
}

/* Local Matrix-Vector Multiplication for Dense */
void dense_mat_vec_mult(int *local_matrix, int *x, int *local_y, int local_N, int N) {
    for (int i = 0; i < local_N; i++) {
        int sum = 0;
        for (int j = 0; j < N; j++) {
            sum += local_matrix[i * N + j] * x[j];
        }
        local_y[i] = sum;
    }
}