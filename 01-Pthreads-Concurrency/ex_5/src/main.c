#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#include "cond_barrier.h"
#include "sense_barrier.h"
#include "pthread_barrier_wrapper.h"

// ===== Select barrier implementation =====
#if defined(COND_BARRIER)
    #define BARRIER_TYPE cond_barrier_t
    #define BARRIER_INIT(b,n) cond_barrier_init(&(b), n)
    #define BARRIER_WAIT(b) cond_barrier_wait(&(b))
    #define BARRIER_DESTROY(b) cond_barrier_destroy(&(b))
#elif defined(SENSE_BARRIER)
    #define BARRIER_TYPE sense_barrier_t
    #define BARRIER_INIT(b,n) sense_barrier_init(&(b), NULL, n)
    #define BARRIER_WAIT(b) sense_barrier_wait(&(b))
    #define BARRIER_DESTROY(b) sense_barrier_destroy(&(b))
#else // default to PTHREAD_BARRIER
    #define BARRIER_TYPE pthread_barrier_t
    #define BARRIER_INIT(b,n) pthread_barrier_init_wrapper(&(b), n)
    #define BARRIER_WAIT(b) pthread_barrier_wait_wrapper(&(b))
    #define BARRIER_DESTROY(b) pthread_barrier_destroy_wrapper(&(b))
#endif

typedef struct {
    long thread_id;
    int iterations;
    BARRIER_TYPE* barrier;
    bool local_sense;
} thread_arg_t;

void* thread_work(void* arg) {
    thread_arg_t* t_arg = (thread_arg_t*)arg;
    int N = t_arg->iterations;
    BARRIER_TYPE* barrier = t_arg->barrier;

    for (int i = 0; i < N; i++) {
        BARRIER_WAIT(*barrier);
    }

    return NULL;
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_threads> <num_iterations>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int num_threads = atoi(argv[1]);
    int num_iterations = atoi(argv[2]);

    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    thread_arg_t* args = malloc(num_threads * sizeof(thread_arg_t));

    BARRIER_TYPE barrier;
    BARRIER_INIT(barrier, num_threads);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (long i = 0; i < num_threads; i++) {
        args[i].thread_id = i;
        args[i].iterations = num_iterations;
        args[i].barrier = &barrier;
        pthread_create(&threads[i], NULL, thread_work, &args[i]);
    }

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    BARRIER_DESTROY(barrier);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
    printf("Total time: %.6f seconds\n", elapsed);

    free(threads);
    free(args);

    return 0;
}
