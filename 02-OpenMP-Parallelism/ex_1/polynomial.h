#ifndef POLYNOMIAL_H
#define POLYNOMIAL_H

#include <pthread.h>

#define M_PI 3.14159265358979323846

typedef int* polynomial;

// ==================== Basic Polynomial Utilities ====================
polynomial polynomial_create(int degree);
void polynomial_destroy(polynomial p);
int polynomial_compare(polynomial p1, polynomial p2, int n);
void polynomial_print(polynomial p, int degree);

// ==================== Polynomial Generation ====================
polynomial polynomial_random(int degree);


// ==================== Polynomial Multiplication Algorithms ====================
polynomial polynomial_naive_sequential(polynomial p1, polynomial p2, int n);
polynomial polynomial_naive_parallel(polynomial a, polynomial b, int n, int T);

#endif
