#ifndef SPARSE_H
#define SPARSE_H

int **sparse_matrix_initialize(int N, double sparsity);
void sparse_matrix_free(int **matrix, int N);
void sparse_matrix_print(int **matrix, int N);
int *sparse_matrix_multiply_vector(int **matrix, int N, int *vector);
int *sparse_matrix_multiply_vector_parallel(int **matrix, int N, int *vector);

#endif
