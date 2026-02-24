#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_AMOUNT 1000000

typedef struct {
    int *balances;
    int numAccounts;
    int numTransactions;
    int percentBalanceQueries;

    int granularity;   // 0 = coarse, 1 = fine
    int lockType;      // 0 = mutex, 1 = rwlock
    int sleepFlag;     // 0 = no sleep, 1 = sleep inside critical section

    pthread_mutex_t *coarseMutex;
    pthread_mutex_t *mutexes;

    pthread_rwlock_t *coarseRW;
    pthread_rwlock_t *rwlocks;

    long long sumRead;

    uint32_t rng_state;
} ThreadData;

/* xorshift32 PRNG */
static inline uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

static inline int rand_range(uint32_t *state, int n) {
    return (int)(xorshift32(state) % (uint32_t)n);
}

static double get_time_monotonic() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void *thread_func(void *arg) {
    ThreadData *d = (ThreadData *)arg;

    uint32_t seed = (uint32_t)(time(NULL) ^ (uintptr_t)pthread_self() ^ (d->numTransactions));
    if (seed == 0) seed = 0xA5A5A5A5;
    d->rng_state = seed;

    for (int i = 0; i < d->numTransactions; i++) {
        int isQuery = (rand_range(&d->rng_state, 100) < d->percentBalanceQueries);

        if (isQuery) {
            int acc = rand_range(&d->rng_state, d->numAccounts);
            int balance = 0;

            if (d->granularity == 0) { // coarse
                if (d->lockType == 0) {
                    pthread_mutex_lock(d->coarseMutex);
                    if (d->sleepFlag) usleep(5); 
                    balance = d->balances[acc];
                    pthread_mutex_unlock(d->coarseMutex);
                } else {
                    pthread_rwlock_rdlock(d->coarseRW);
                    if (d->sleepFlag) usleep(5); 
                    balance = d->balances[acc];
                    pthread_rwlock_unlock(d->coarseRW);
                }
            } else { // fine
                if (d->lockType == 0) {
                    pthread_mutex_lock(&d->mutexes[acc]);
                    if (d->sleepFlag) usleep(5); 
                    balance = d->balances[acc];
                    pthread_mutex_unlock(&d->mutexes[acc]);
                } else {
                    pthread_rwlock_rdlock(&d->rwlocks[acc]);
                    if (d->sleepFlag) usleep(5); 
                    balance = d->balances[acc];
                    pthread_rwlock_unlock(&d->rwlocks[acc]);
                }
            }

            d->sumRead += balance;
        } else {
            int from = rand_range(&d->rng_state, d->numAccounts);
            int to;
            do { to = rand_range(&d->rng_state, d->numAccounts); } while (to == from);
            int amount = rand_range(&d->rng_state, MAX_AMOUNT / 100);

            if (d->granularity == 0) {
                if (d->lockType == 0) {
                    pthread_mutex_lock(d->coarseMutex);
                    if (d->sleepFlag) usleep(10); 
                    if (d->balances[from] >= amount) {
                        d->balances[from] -= amount;
                        d->balances[to] += amount;
                    }
                    pthread_mutex_unlock(d->coarseMutex);
                } else {
                    pthread_rwlock_wrlock(d->coarseRW);
                    if (d->sleepFlag) usleep(10); 
                    if (d->balances[from] >= amount) {
                        d->balances[from] -= amount;
                        d->balances[to] += amount;
                    }
                    pthread_rwlock_unlock(d->coarseRW);
                }
            } else {
                int first = from < to ? from : to;
                int second = from < to ? to : from;

                if (d->lockType == 0) {
                    pthread_mutex_lock(&d->mutexes[first]);
                    pthread_mutex_lock(&d->mutexes[second]);
                    if (d->sleepFlag) usleep(10); 
                } else {
                    pthread_rwlock_wrlock(&d->rwlocks[first]);
                    pthread_rwlock_wrlock(&d->rwlocks[second]);
                    if (d->sleepFlag) usleep(10); 
                }

                if (d->balances[from] >= amount) {
                    d->balances[from] -= amount;
                    d->balances[to] += amount;
                }

                if (d->lockType == 0) {
                    pthread_mutex_unlock(&d->mutexes[second]);
                    pthread_mutex_unlock(&d->mutexes[first]);
                } else {
                    pthread_rwlock_unlock(&d->rwlocks[second]);
                    pthread_rwlock_unlock(&d->rwlocks[first]);
                }
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 8) {
        fprintf(stderr,
            "Usage: %s <num_accounts> <num_transactions> <percent_queries> "
            "<granularity:0=coarse,1=fine> <lockType:0=mutex,1=rwlock> <num_threads> <sleep_flag:0/1>\n",
            argv[0]);
        return EXIT_FAILURE;
    }

    int numAccounts = atoi(argv[1]);
    int numTransactions = atoi(argv[2]);
    int percentQueries = atoi(argv[3]);
    int granularity = atoi(argv[4]);
    int lockType = atoi(argv[5]);
    int numThreads = atoi(argv[6]);
    int sleepFlag = atoi(argv[7]);

    if (numAccounts <= 0 || numTransactions <= 0 || percentQueries < 0 || percentQueries > 100 || numThreads <= 0 || (sleepFlag !=0 && sleepFlag !=1)) {
        fprintf(stderr, "Invalid arguments.\n");
        return EXIT_FAILURE;
    }

    int *balances = malloc(numAccounts * sizeof(int));
    if (!balances) { perror("malloc"); return EXIT_FAILURE; }

    uint32_t s = (uint32_t)time(NULL);
    for (int i = 0; i < numAccounts; i++) {
        s = xorshift32(&s);
        balances[i] = (int)(s % MAX_AMOUNT);
    }

    pthread_mutex_t coarseMutex;
    pthread_mutex_t *mutexes = NULL;
    pthread_rwlock_t coarseRW;
    pthread_rwlock_t *rwlocks = NULL;

    if (granularity == 0) {
        if (lockType == 0) pthread_mutex_init(&coarseMutex, NULL);
        else pthread_rwlock_init(&coarseRW, NULL);
    } else {
        if (lockType == 0) {
            mutexes = calloc(numAccounts, sizeof(pthread_mutex_t));
            for (int i = 0; i < numAccounts; i++) pthread_mutex_init(&mutexes[i], NULL);
        } else {
            rwlocks = calloc(numAccounts, sizeof(pthread_rwlock_t));
            for (int i = 0; i < numAccounts; i++) pthread_rwlock_init(&rwlocks[i], NULL);
        }
    }

    pthread_t *threads = malloc(numThreads * sizeof(pthread_t));
    ThreadData *td = calloc(numThreads, sizeof(ThreadData));

    double tstart = get_time_monotonic();

    for (int i = 0; i < numThreads; i++) {
        td[i].balances = balances;
        td[i].numAccounts = numAccounts;
        td[i].numTransactions = numTransactions;
        td[i].percentBalanceQueries = percentQueries;
        td[i].granularity = granularity;
        td[i].lockType = lockType;
        td[i].sleepFlag = sleepFlag;

        td[i].coarseMutex = (granularity == 0 && lockType == 0) ? &coarseMutex : NULL;
        td[i].coarseRW = (granularity == 0 && lockType == 1) ? &coarseRW : NULL;
        td[i].mutexes = (granularity == 1 && lockType == 0) ? mutexes : NULL;
        td[i].rwlocks = (granularity == 1 && lockType == 1) ? rwlocks : NULL;
        td[i].sumRead = 0;
        td[i].rng_state = 0;

        int rc = pthread_create(&threads[i], NULL, thread_func, &td[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create failed: %d\n", rc);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
        printf("Thread %d sum read: %.2f€\n", i, td[i].sumRead / 100.0);
    }

    double tend = get_time_monotonic();

    long long total = 0;
    for (int i = 0; i < numAccounts; i++) total += balances[i];

    printf("\nTotal balance: %.2f€\n", total / 100.0);
    printf("Exec time: %.6f seconds\n", tend - tstart);

    if (granularity == 0) {
        if (lockType == 0) pthread_mutex_destroy(&coarseMutex);
        else pthread_rwlock_destroy(&coarseRW);
    } else {
        if (lockType == 0) {
            for (int i = 0; i < numAccounts; i++) pthread_mutex_destroy(&mutexes[i]);
            free(mutexes);
        } else {
            for (int i = 0; i < numAccounts; i++) pthread_rwlock_destroy(&rwlocks[i]);
            free(rwlocks);
        }
    }

    free(td);
    free(threads);
    free(balances);

    return 0;
}
