#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "csr.h"

CSR* csr_allocate(int nnz, int N) {
    CSR* csr = (CSR*)malloc(sizeof(CSR));
    if (!csr) exit(EXIT_FAILURE);

    csr->values = (int*)malloc(nnz * sizeof(int));
    csr->indexes = (int*)malloc(nnz * sizeof(int));
    csr->row_ptr = (int*)malloc((N + 1) * sizeof(int));
    csr->nnz = nnz;

    if (!csr->values || !csr->indexes || !csr->row_ptr) {
        fprintf(stderr, "CSR allocation failed\n");
        exit(EXIT_FAILURE);
    }

    return csr;
}

void csr_destroy(CSR* csr) {
    if (!csr) return;
    free(csr->values);
    free(csr->indexes);
    free(csr->row_ptr);
    free(csr);
}

void csr_print(CSR* csr, int N) {
    printf("Values: ");
    for (int i = 0; i < csr->nnz; i++)
        printf("%d ", csr->values[i]);

    printf("\nIndexes: ");
    for (int i = 0; i < csr->nnz; i++)
        printf("%d ", csr->indexes[i]);

    printf("\nRow Ptr: ");
    for (int i = 0; i <= N; i++)
        printf("%d ", csr->row_ptr[i]);

    printf("\n");
}

/* --- Serial CSR conversion --- */
CSR* sparse_matrix_to_csr(int **matrix, int N) {
    int *row_nnz = (int*)malloc(N * sizeof(int));
    if (!row_nnz) exit(EXIT_FAILURE);

    int nnz = 0;
    for (int i = 0; i < N; i++) {
        int c = 0;
        for (int j = 0; j < N; j++)
            if (matrix[i][j] != 0) c++;
        row_nnz[i] = c;
        nnz += c;
    }

    CSR* csr = csr_allocate(nnz, N);

    // fill row_ptr
    int sum = 0;
    for (int i = 0; i < N; i++) {
        csr->row_ptr[i] = sum;
        sum += row_nnz[i];
    }
    csr->row_ptr[N] = nnz;

    // fill values/indexes
    for (int i = 0; i < N; i++) {
        int idx = csr->row_ptr[i];
        for (int j = 0; j < N; j++) {
            if (matrix[i][j] != 0) {
                csr->values[idx] = matrix[i][j];
                csr->indexes[idx] = j;
                idx++;
            }
        }
    }

    free(row_nnz);
    return csr;
}

/* --- Parallel CSR conversion --- */
CSR* sparse_matrix_to_csr_parallel(int **matrix, int N) {
    int *row_nnz = (int*)malloc(N * sizeof(int));
    if (!row_nnz) exit(EXIT_FAILURE);

    int nnz = 0;

    // Phase 1: count non-zeros per row (parallel safe)
    #pragma omp parallel for reduction(+:nnz)
    for (int i = 0; i < N; i++) {
        int c = 0;
        for (int j = 0; j < N; j++)
            if (matrix[i][j] != 0) c++;
        row_nnz[i] = c;
        nnz += c;
    }

    CSR* csr = csr_allocate(nnz, N);

    // Phase 2: prefix sum for row_ptr
    int sum = 0;
    for (int i = 0; i < N; i++) {
        csr->row_ptr[i] = sum;
        sum += row_nnz[i];
    }
    csr->row_ptr[N] = nnz;

    // Phase 3: fill values/indexes (parallel safe because idx unique per row)
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        int idx = csr->row_ptr[i];
        for (int j = 0; j < N; j++) {
            if (matrix[i][j] != 0) {
                csr->values[idx] = matrix[i][j];
                csr->indexes[idx] = j;
                idx++;
            }
        }
    }

    free(row_nnz);
    return csr;
}

/* --- Serial CSR mat-vec --- */
int* csr_multiply_vector(CSR* csr, int N, int *vector) {
    int *result = (int*)calloc(N, sizeof(int));
    if (!result) exit(EXIT_FAILURE);

    for (int i = 0; i < N; i++)
        for (int j = csr->row_ptr[i]; j < csr->row_ptr[i+1]; j++)
            result[i] += csr->values[j] * vector[csr->indexes[j]];

    return result;
}

/* --- Parallel CSR mat-vec --- */
int* csr_multiply_vector_parallel(CSR* csr, int N, int *vector) {
    int *result = (int*)calloc(N, sizeof(int));
    if (!result) exit(EXIT_FAILURE);

    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        int sum = 0;
        for (int j = csr->row_ptr[i]; j < csr->row_ptr[i+1]; j++)
            sum += csr->values[j] * vector[csr->indexes[j]];
        result[i] = sum;
    }

    return result;
}
