#ifndef CSR_H
#define CSR_H

typedef struct CSR_struct CSR;

struct CSR_struct {
    int *values;
    int *indexes;
    int *row_ptr;
    int nnz;
};

CSR* csr_allocate(int nnz, int N);
void csr_destroy(CSR* csr);
void csr_print(CSR* csr, int N);

CSR* sparse_matrix_to_csr(int **matrix, int N);
CSR* sparse_matrix_to_csr_parallel(int **matrix, int N);

int* csr_multiply_vector(CSR* csr, int N, int *vector);
int* csr_multiply_vector_parallel(CSR* csr, int N, int *vector);

#endif
