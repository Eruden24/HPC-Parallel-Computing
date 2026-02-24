
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>  
#include "polynomial.h"

// ==================== Basic Polynomial Utilities ====================
polynomial polynomial_create(int degree) {
    polynomial p = calloc((size_t)(degree + 1), sizeof(int));
    if (!p) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return p;
}

void polynomial_destroy(polynomial p) {
    free(p);
}

int polynomial_compare(polynomial p1, polynomial p2, int n) {
    for (int i = 0; i <= 2*n; i++) { 
        if (p1[i] != p2[i]) return 0;
    }
    return 1;
}

void polynomial_print(polynomial p, int degree) {
    for (int i = degree; i >= 0; i--) {
        if (i < degree) printf(" %c ", p[i] < 0 ? '-' : '+');
        if (i > 0) {
            printf("%d", abs(p[i]));
            if (i > 1) printf("x^%d", i);
            else printf("x");
        } else printf("%d", abs(p[i]));
    }
    printf("\n\n");
}

// ==================== Polynomial Generation ====================
polynomial polynomial_random(int degree) {
    polynomial p = polynomial_create(degree);
    for (int i = 0; i <= degree; i++) {
        int sign = (rand() % 2) ? 1 : -1;
        p[i] = sign * ((rand() % 99) + 1);
    }
    return p;
}


// ==================== Polynomial Multiplication Algorithms ====================

// Naive Sequential Multiplication

polynomial polynomial_naive_sequential(polynomial p1, polynomial p2, int n) {
    polynomial res = polynomial_create(2*n);
    for (int i = 0; i <= n; i++)
        for (int j = 0; j <= n; j++)
            res[i+j] += p1[i]*p2[j];
    return res;
}

// Naive Parallel Multiplication

typedef struct {
    polynomial a;
    polynomial b;
    polynomial result;
    int n;
    int k_start;
    int k_end;
} diag_args_t;

void *diag_thread(void *arg) {
    diag_args_t *args = (diag_args_t*)arg;
    int n = args->n;

    for (int k = args->k_start; k <= args->k_end; k++) {

        long long sum = 0;

        int i_start = (k < n ? 0 : k - n);
        int i_end   = (k < n ? k : n);

        for (int i = i_start; i <= i_end; i++) {
            sum += args->a[i] * args->b[k - i];
        }

        args->result[k] = sum;
    }

    free(arg);
    return NULL;
}

polynomial polynomial_naive_parallel(polynomial a, polynomial b, int n, int T) {
    int total_diag = 2 * n + 1;

    clock_t start = clock();  // start timing preprocessing

    // compute prefix sum of diagonal sizes
    long long *prefix = malloc(sizeof(long long) * total_diag);
    for (int k = 0; k <= 2*n; k++) {
        long long diag_size = (k <= n ? (k + 1) : (2*n - k + 1));
        prefix[k] = (k == 0 ? diag_size : prefix[k-1] + diag_size);
    }

    long long total_work = prefix[2*n];
    long long work_per_thread = total_work / T;

    // optional: store k_start/k_end per thread (could do later)
    int *k_start = malloc(sizeof(int) * T);
    int *k_end   = malloc(sizeof(int) * T);

    int k_prev = 0;
    for (int t = 0; t < T; t++) {
        long long target = (t == T-1 ? total_work : (t+1) * work_per_thread);
        int k_e = k_prev;
        while (k_e <= 2*n && prefix[k_e] < target)
            k_e++;

        k_start[t] = k_prev;
        k_end[t] = k_e;
        k_prev = k_e + 1;
    }

    clock_t end = clock();  // end timing preprocessing
    double preprocessing_time = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Preprocessing time: %f s\n", preprocessing_time);

    // =========================
    // start threads
    // =========================
    pthread_t *threads = malloc(sizeof(pthread_t) * T);
    polynomial result = polynomial_create(2*n);

    for (int t = 0; t < T; t++) {
        diag_args_t *args = malloc(sizeof(diag_args_t));
        args->a = a;
        args->b = b;
        args->result = result;
        args->n = n;
        args->k_start = k_start[t];
        args->k_end   = k_end[t];

        pthread_create(&threads[t], NULL, diag_thread, args);
    }

    for (int t = 0; t < T; t++)
        pthread_join(threads[t], NULL);

    free(prefix);
    free(k_start);
    free(k_end);
    free(threads);

    return result;
}
