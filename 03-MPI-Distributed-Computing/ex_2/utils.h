#ifndef UTILS_H
#define UTILS_H

#include <mpi.h>

// CSR Structure (Local or Global)
typedef struct {
    int *values;
    int *col_ind;
    int *row_ptr;
    int nnz;
    int num_rows;
} CSR;

// Helper functions
int* generate_dense_matrix(int N, double sparsity);
int* generate_vector(int N);
CSR* dense_to_csr(int *matrix, int N);
void free_csr(CSR *csr);

// Local computation functions
void csr_mat_vec_mult(CSR *local_csr, int *x, int *local_y, int local_N);
void dense_mat_vec_mult(int *local_matrix, int *x, int *local_y, int local_N, int N);

#endif