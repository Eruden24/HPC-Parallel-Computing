#include "sense_barrier.h"

void sense_barrier_init(sense_barrier_t* b, pthread_mutexattr_t* attr, int num_threads) {
    pthread_mutex_init(&(b->lock), attr);
    b->thread_barrier_number = num_threads;
    b->total_thread = 0;
    b->flag = false;  // initial global sense
}

void sense_barrier_wait(sense_barrier_t* b) {

    bool local_sense = b->flag;
    if(!pthread_mutex_lock(&(b->lock))){
        b->total_thread += 1;
        local_sense = !local_sense;
        
        if (b->total_thread == b->thread_barrier_number){
            b->total_thread = 0;
            b->flag = local_sense;
            pthread_mutex_unlock(&(b->lock));
        } else {
            pthread_mutex_unlock(&(b->lock));
            while (b->flag != local_sense); // wait for flag
        }
    }
}

void sense_barrier_destroy(sense_barrier_t* b) {
    pthread_mutex_destroy(&b->lock);
}
