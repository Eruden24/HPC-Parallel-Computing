#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include "pthread_barrier_wrapper.h"

void pthread_barrier_init_wrapper(pthread_barrier_t* b, int num_threads) {
    pthread_barrier_init(b, NULL, num_threads);
}

void pthread_barrier_wait_wrapper(pthread_barrier_t* b) {
    pthread_barrier_wait(b);
}

void pthread_barrier_destroy_wrapper(pthread_barrier_t* b) {
    pthread_barrier_destroy(b);
}
