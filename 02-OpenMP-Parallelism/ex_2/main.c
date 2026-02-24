	#include <stdio.h>
	#include <stdlib.h>
	#include <time.h>
	#include <omp.h>
	#include <string.h>
	#include "csr.h"
	#include "sparse.h"

	int main(int argc, char* argv[]) {

	if (argc < 5) {
		printf("Usage:\n");
		printf("  %s <N> <sparsity> serial <iterations>\n", argv[0]);
		printf("  %s <N> <sparsity> parallel <threads> <iterations>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int N = atoi(argv[1]);
	double sparsity = atof(argv[2]);
	char* mode = argv[3];

	if (sparsity < 0.0 || sparsity > 1.0) {
		fprintf(stderr, "Error: sparsity must be between 0 and 1\n");
		return EXIT_FAILURE;
	}

	int num_threads = 1;
	int M = 1;

	if (strcmp(mode, "serial") == 0) {
		M = atoi(argv[4]);
	} else if (strcmp(mode, "parallel") == 0) {
		if (argc != 6) {
			fprintf(stderr, "Parallel mode requires <threads> <iterations>\n");
			return EXIT_FAILURE;
		}
		num_threads = atoi(argv[4]);
		M = atoi(argv[5]);
		omp_set_num_threads(num_threads);
	} else {
		fprintf(stderr, "Mode must be 'serial' or 'parallel'\n");
		return EXIT_FAILURE;
	}

	printf("Matrix size: %dx%d, Sparsity: %.2f, Mode: %s\n",
			N, N, sparsity, mode);
	printf("Threads: %d, Iterations: %d\n", num_threads, M);

	unsigned int seed = (unsigned int)time(NULL);

	/* --- Create sparse matrix and vector --- */
	int **matrix = sparse_matrix_initialize(N, sparsity);
	int *vector = (int*) malloc(N * sizeof(int));
	if (!vector) exit(EXIT_FAILURE);

	for (int i = 0; i < N; i++)
		vector[i] = rand_r(&seed) % 10;

	/* --- CSR Construction --- */
	double t_start = omp_get_wtime();
	CSR *csr = (strcmp(mode, "parallel") == 0)
		? sparse_matrix_to_csr_parallel(matrix, N)
		: sparse_matrix_to_csr(matrix, N);
	double t_end = omp_get_wtime();
	double csr_build_time = t_end - t_start;

	/* --- Dense vs CSR mat-vec --- */
	int *x, *y;
	double csr_total = 0.0, dense_total = 0.0;

	/* CSR mat-vec iterations */
	x = (int*) malloc(N * sizeof(int));
	memcpy(x, vector, N * sizeof(int));

	for (int iter = 0; iter < M; iter++) {
		t_start = omp_get_wtime();
		y = (strcmp(mode, "parallel") == 0)
			? csr_multiply_vector_parallel(csr, N, x)
			: csr_multiply_vector(csr, N, x);
		t_end = omp_get_wtime();
		csr_total += (t_end - t_start);

		free(x);
		x = y;
	}
	int *csr_final = x;

	/* Dense mat-vec iterations */
	x = (int*) malloc(N * sizeof(int));
	memcpy(x, vector, N * sizeof(int));

	for (int iter = 0; iter < M; iter++) {
		t_start = omp_get_wtime();
		y = (strcmp(mode, "parallel") == 0)
			? sparse_matrix_multiply_vector_parallel(matrix, N, x)
			: sparse_matrix_multiply_vector(matrix, N, x);
		t_end = omp_get_wtime();
		dense_total += (t_end - t_start);

		free(x);
		x = y;
	}
	int *dense_final = x;

	/* --- Validation --- */
	for (int i = 0; i < N; i++) {
		if (dense_final[i] != csr_final[i]) {
			fprintf(stderr, "Validation failed at index %d\n", i);
			return EXIT_FAILURE;
		}
	}
	printf("Validation: OK\n");

	/* --- Print results --- */
	printf("\nTiming Results:\n");
	printf("CSR construction time       : %.6f s\n", csr_build_time);
	printf("CSR mat-vec total time      : %.6f s\n", csr_total);
	printf("Dense mat-vec total time    : %.6f s\n", dense_total);

	/* --- Cleanup --- */
	free(vector);
	free(csr_final);
	free(dense_final);
	sparse_matrix_free(matrix, N);
	csr_destroy(csr);

	return 0;
}
