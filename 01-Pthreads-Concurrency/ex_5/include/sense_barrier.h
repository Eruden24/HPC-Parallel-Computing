#ifndef SENSE_BARRIER_H
#define SENSE_BARRIER_H

#include <stdbool.h>
#include <pthread.h>

typedef struct {
    int thread_barrier_number;
    int total_thread;
    volatile bool flag;    // volatile to prevent compiler optimizations
    pthread_mutex_t lock;
} sense_barrier_t;


void sense_barrier_init(sense_barrier_t* barrier, pthread_mutexattr_t* mutex_attr, int num_threads);
void sense_barrier_wait(sense_barrier_t* barrier);
void sense_barrier_destroy(sense_barrier_t* barrier);

#endif
