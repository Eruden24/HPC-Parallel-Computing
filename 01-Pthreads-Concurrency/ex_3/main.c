#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

struct array_stats_plain {
    long long int info_array_0;
    long long int info_array_1;
    long long int info_array_2;
    long long int info_array_3;
};

struct array_stats_padded {
    long long int info_array_0;
    char pad0[64 - sizeof(long long int)];
    long long int info_array_1;
    char pad1[64 - sizeof(long long int)];
    long long int info_array_2;
    char pad2[64 - sizeof(long long int)];
    long long int info_array_3;
    char pad3[64 - sizeof(long long int)];
};

void *array_stats_ptr;
int use_padding = 0; // 0 = plain, 1 = padded

struct thread_arg_s {
    int idx;
    int *array;
    int size;
};

void* count_nonzero_thread(void* arg) {
    struct thread_arg_s* t_arg = (struct thread_arg_s*) arg;
    for (int j = 0; j < t_arg->size; j++) {
        if (t_arg->array[j] != 0) {
            if (use_padding) {
                struct array_stats_padded *stats = (struct array_stats_padded*)array_stats_ptr;
                switch (t_arg->idx) {
                    case 0: stats->info_array_0++; break;
                    case 1: stats->info_array_1++; break;
                    case 2: stats->info_array_2++; break;
                    case 3: stats->info_array_3++; break;
                }
            } else {
                struct array_stats_plain *stats = (struct array_stats_plain*)array_stats_ptr;
                switch (t_arg->idx) {
                    case 0: stats->info_array_0++; break;
                    case 1: stats->info_array_1++; break;
                    case 2: stats->info_array_2++; break;
                    case 3: stats->info_array_3++; break;
                }
            }
        }
    }
    free(t_arg);
    return NULL;
}

void count_nonzero_seq(int **arrays, int size) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < size; j++) {
            if (arrays[i][j] != 0) {
                if (use_padding) {
                    struct array_stats_padded *stats = (struct array_stats_padded*)array_stats_ptr;
                    switch (i) {
                        case 0: stats->info_array_0++; break;
                        case 1: stats->info_array_1++; break;
                        case 2: stats->info_array_2++; break;
                        case 3: stats->info_array_3++; break;
                    }
                } else {
                    struct array_stats_plain *stats = (struct array_stats_plain*)array_stats_ptr;
                    switch (i) {
                        case 0: stats->info_array_0++; break;
                        case 1: stats->info_array_1++; break;
                        case 2: stats->info_array_2++; break;
                        case 3: stats->info_array_3++; break;
                    }
                }
            }
        }
    }
}

void count_nonzero_par(int **arrays, int size) {
    pthread_t threads[4];
    for (int i = 0; i < 4; i++) {
        struct thread_arg_s* arg = malloc(sizeof(struct thread_arg_s));
        arg->idx = i;
        arg->array = arrays[i];
        arg->size = size;
        pthread_create(&threads[i], NULL, count_nonzero_thread, arg);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <array_size> <padding 0 or 1>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int size = strtol(argv[1], NULL, 10);
    use_padding = strtol(argv[2], NULL, 10);

    srand(0);
    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    int **arrays = malloc(4 * sizeof(int*));
    for (int i = 0; i < 4; i++) {
        arrays[i] = malloc(size * sizeof(int));
        for (int j = 0; j < size; j++) {
            arrays[i][j] = rand() % 10;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double alloc_init_time = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_nsec - t_start.tv_nsec)/1e9;

    if (use_padding) {
        array_stats_ptr = calloc(1, sizeof(struct array_stats_padded));
    } else {
        array_stats_ptr = calloc(1, sizeof(struct array_stats_plain));
    }

    clock_gettime(CLOCK_MONOTONIC, &t_start);
    count_nonzero_seq(arrays, size);
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double seq_time = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_nsec - t_start.tv_nsec)/1e9;

    long long int seq_results[4];
    if (use_padding) {
        struct array_stats_padded *stats = (struct array_stats_padded*)array_stats_ptr;
        seq_results[0] = stats->info_array_0;
        seq_results[1] = stats->info_array_1;
        seq_results[2] = stats->info_array_2;
        seq_results[3] = stats->info_array_3;
        stats->info_array_0 = stats->info_array_1 = stats->info_array_2 = stats->info_array_3 = 0;
    } else {
        struct array_stats_plain *stats = (struct array_stats_plain*)array_stats_ptr;
        seq_results[0] = stats->info_array_0;
        seq_results[1] = stats->info_array_1;
        seq_results[2] = stats->info_array_2;
        seq_results[3] = stats->info_array_3;
        stats->info_array_0 = stats->info_array_1 = stats->info_array_2 = stats->info_array_3 = 0;
    }

    clock_gettime(CLOCK_MONOTONIC, &t_start);
    count_nonzero_par(arrays, size);
    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double par_time = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_nsec - t_start.tv_nsec)/1e9;

    int correct;
    if (use_padding) {
        struct array_stats_padded *stats = (struct array_stats_padded*)array_stats_ptr;
        correct = (seq_results[0] == stats->info_array_0 &&
                   seq_results[1] == stats->info_array_1 &&
                   seq_results[2] == stats->info_array_2 &&
                   seq_results[3] == stats->info_array_3);
    } else {
        struct array_stats_plain *stats = (struct array_stats_plain*)array_stats_ptr;
        correct = (seq_results[0] == stats->info_array_0 &&
                   seq_results[1] == stats->info_array_1 &&
                   seq_results[2] == stats->info_array_2 &&
                   seq_results[3] == stats->info_array_3);
    }

    printf("Allocation & initialization time: %f seconds\n", alloc_init_time);
    printf("Sequential execution time: %f seconds\n", seq_time);
    printf("Parallel execution time: %f seconds\n", par_time);
    printf("Parallel results match sequential: %s\n", correct ? "YES" : "NO");

    for (int i = 0; i < 4; i++) free(arrays[i]);
    free(arrays);
    free(array_stats_ptr);

    return 0;
}
