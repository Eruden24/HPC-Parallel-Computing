#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "array.h"

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  Serial:   %s <array_size> serial\n", argv[0]);
        fprintf(stderr, "  Parallel: %s <array_size> parallel <num_threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int N = atoi(argv[1]);
    char* mode = argv[2];
    int num_threads = 1;

    if (strcmp(mode, "parallel") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Error: Parallel mode requires <num_threads>\n");
            return EXIT_FAILURE;
        }
        num_threads = atoi(argv[3]);
    } else if (strcmp(mode, "serial") != 0) {
        fprintf(stderr, "Unknown mode: %s\n", mode);
        return EXIT_FAILURE;
    }

    int* array = array_allocate(N);
    array_init_random(array, N);

    double start_time = omp_get_wtime();

    if (strcmp(mode, "serial") == 0) {
        array_merge_sort(array, 0, N - 1);
    } else {
        array_merge_sort_parallel(array, N, num_threads);
    }

    double end_time = omp_get_wtime();

    printf("Sorted correctly? %s\n", array_is_sorted(array, N) ? "Yes" : "No");
    printf("Time elapsed: %.6f seconds\n", end_time - start_time);

    array_free(array);
    return 0;
}
