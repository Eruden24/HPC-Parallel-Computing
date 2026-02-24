#ifndef PTHREAD_BARRIER_WRAPPER_H
#define PTHREAD_BARRIER_WRAPPER_H

#include <pthread.h>

void pthread_barrier_init_wrapper(pthread_barrier_t* b, int num_threads);
void pthread_barrier_wait_wrapper(pthread_barrier_t* b);
void pthread_barrier_destroy_wrapper(pthread_barrier_t* b);

#endif
