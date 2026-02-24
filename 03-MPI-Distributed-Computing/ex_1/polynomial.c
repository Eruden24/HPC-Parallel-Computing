#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "polynomial.h"

/* ==================== Basic Polynomial Utilities ==================== */

polynomial polynomial_create(int degree)
{
    polynomial p = calloc((size_t)(degree + 1), sizeof(long long));
    if (!p) { perror("calloc"); exit(EXIT_FAILURE); }
    return p;
}

void polynomial_destroy(polynomial p)
{
    free(p);
}

int polynomial_compare(polynomial p1, polynomial p2, int n)
{
    int size = 2 * n + 1;
    for (int i = 0; i < size; i++) {
        if (p1[i] != p2[i]) return 0;
    }
    return 1;
}

void polynomial_print(polynomial poly, int degree)
{
    if (degree > 20) {
        printf("Polynomial is too long to print (degree %d)\n", degree);
        return;
    }
    for (int i = 0; i <= degree; i++) {
        if (i == 0) printf("%lld", poly[i]);
        else printf(" + %lld*x^%d", poly[i], i);
    }
    printf("\n");
}

/* ==================== Polynomial Generation ==================== */

polynomial polynomial_random(int degree)
{
    polynomial p = polynomial_create(degree);
    for (int i = 0; i <= degree; i++) {
        long long sign = (rand() & 1) ? 1 : -1;
        p[i] = sign * ((rand() % 99) + 1);
    }
    return p;
}

/* ==================== Sequential Multiplication ==================== */

polynomial polynomial_naive_sequential(polynomial a, polynomial b, int n)
{
    int res_size = 2 * n + 1;
    polynomial res = calloc(res_size, sizeof(long long));
    if (!res) { perror("calloc"); exit(EXIT_FAILURE); }

    for (int i = 0; i <= n; i++) {
        for (int j = 0; j <= n; j++) {
            res[i + j] += a[i] * b[j];
        }
    }
    return res;
}
