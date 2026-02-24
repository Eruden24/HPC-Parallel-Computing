
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>  
#include <omp.h>
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

polynomial polynomial_naive_parallel(polynomial a, polynomial b, int n, int T) {
    polynomial result = polynomial_create(2 * n);

    #pragma omp parallel for num_threads(T) schedule(static,1)
    for (int k = 0; k <= 2 * n; k++) {

        long long sum = 0;

        int i_start = (k < n ? 0 : k - n);
        int i_end   = (k < n ? k : n);

        for (int i = i_start; i <= i_end; i++) {
            sum += a[i] * b[k - i];
        }

        result[k] = sum;
    }

    return result;
}
