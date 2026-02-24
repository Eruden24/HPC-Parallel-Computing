#include "cond_barrier.h"

void cond_barrier_init(cond_barrier_t* b, int total_threads) {
    pthread_mutex_init(&b->mutex, NULL);
    pthread_cond_init(&b->cond, NULL);
    b->count = 0;
    b->total = total_threads;
    b->generation = 0;
}

void cond_barrier_destroy(cond_barrier_t* b) {
    pthread_mutex_destroy(&b->mutex);
    pthread_cond_destroy(&b->cond);
}

void cond_barrier_wait(cond_barrier_t* b) {
    pthread_mutex_lock(&b->mutex);

    int my_gen = b->generation;
    b->count++;

    if (b->count == b->total) {
        b->count = 0;
        b->generation++;
        pthread_cond_broadcast(&b->cond);
    } else {
        while (my_gen == b->generation)
            pthread_cond_wait(&b->cond, &b->mutex);
    }

    pthread_mutex_unlock(&b->mutex);
}
