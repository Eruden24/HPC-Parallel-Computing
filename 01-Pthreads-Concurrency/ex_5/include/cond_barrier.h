#ifndef COND_BARRIER_T
#define COND_BARRIER_T

#include <pthread.h>

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int total;
    int generation;
} cond_barrier_t;

void cond_barrier_init(cond_barrier_t* b, int total_threads);
void cond_barrier_destroy(cond_barrier_t* b);
void cond_barrier_wait(cond_barrier_t* b);

#endif
