#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t mutex;
pthread_rwlock_t rwlock;

long counter;

void *increase_counter_mutex(void* rank);
void *increase_counter_atomic(void* rank);
void *increase_counter_rw_mutex(void* rank);

void *(*increase_counter[3])(void *) = {
    increase_counter_mutex,
    increase_counter_atomic,
    increase_counter_rw_mutex
};

struct thread_args {
    long rank;
    int iterations;
};

int main(int argc, char * argv[]) {

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <threads> <iterations> <0=mutex | 1=atomic | 2=rwlock>\n", argv[0]);
        return EXIT_FAILURE;
    }

    long thread_count = strtol(argv[1], NULL, 10);
    long iterations = strtol(argv[2], NULL, 10);
    long choice = strtol(argv[3], NULL, 10);

    if (choice < 0 || choice > 2) {
        fprintf(stderr, "Invalid choice. Use 0 for mutex, 1 for atomic, or 2 for rwlock.\n");
        return EXIT_FAILURE;
    }

    pthread_t *thread_handles = malloc(thread_count * sizeof(pthread_t));
    struct thread_args *args = malloc(thread_count * sizeof(struct thread_args));

    if (choice == 0) {
        pthread_mutex_init(&mutex, NULL);
    } else if (choice == 2) {
        pthread_rwlock_init(&rwlock, NULL);
    }

    counter = 0;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (long thread = 0; thread < thread_count; thread++) {
        args[thread].rank = thread;
        args[thread].iterations = iterations;
        pthread_create(&thread_handles[thread], NULL, increase_counter[choice], (void*) &args[thread]);
    }

    for (long thread = 0; thread < thread_count; thread++){
        pthread_join(thread_handles[thread], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("Final counter value: %ld\n", counter);
    printf("Execution time: %.6f seconds\n", elapsed);

    free(thread_handles);

    if (choice == 0) {
        pthread_mutex_destroy(&mutex);
    } else if (choice == 2) {
        pthread_rwlock_destroy(&rwlock);
    }

    return 0;
}

void *increase_counter_mutex(void* arg) {
    struct thread_args *args = (struct thread_args*) arg;

    for (int i = 0; i < args->iterations; i++) {
        pthread_mutex_lock(&mutex);
        counter++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}


void *increase_counter_atomic(void* arg) {
    struct thread_args *args = (struct thread_args*) arg;

    for (int i = 0; i < args->iterations; i++) {
        __atomic_fetch_add(&counter, 1, __ATOMIC_SEQ_CST);
    }
    return NULL;
}

void *increase_counter_rw_mutex(void* arg) {
    struct thread_args *args = (struct thread_args*) arg;

    for (int i = 0; i < args->iterations; i++) {
        pthread_rwlock_wrlock(&rwlock);
        counter++;
        pthread_rwlock_unlock(&rwlock);
    }
    return NULL;
}
