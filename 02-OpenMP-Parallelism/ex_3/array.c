#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include "array.h"

#define THRESHOLD 1000  // below this, we do serial merge sort to avoid too many tasks

int* array_allocate(int N) {
    int* array = (int*) malloc(N * sizeof(int));
    if (!array) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return array;
}

void array_free(int* array) {
    free(array);
}

void array_init_random(int* array, int N) {
    unsigned int seed = 12345; // deterministic seed
    for (int i = 0; i < N; i++) {
        array[i] = rand_r(&seed) % 100;
    }
}

void array_print(int* array, int N) {
    for (int i = 0; i < N; i++) {
        printf("%d ", array[i]);
    }
    printf("\n");
}

int array_is_sorted(int* array, int N) {
    for (int i = 1; i < N; i++) {
        if (array[i - 1] > array[i]) return 0;
    }
    return 1;
}

// Serial merge sort
void array_merge_sort(int arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        array_merge_sort(arr, left, mid);
        array_merge_sort(arr, mid + 1, right);

        int n1 = mid - left + 1;
        int n2 = right - mid;
        int* L = (int*)malloc(n1 * sizeof(int));
        int* R = (int*)malloc(n2 * sizeof(int));

        for (int i = 0; i < n1; i++) L[i] = arr[left + i];
        for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
            arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
        }
        while (i < n1) arr[k++] = L[i++];
        while (j < n2) arr[k++] = R[j++];

        free(L);
        free(R);
    }
}

void array_merge_sort_parallel_task(int* arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        // If subarray is small, sort serially
        if (right - left < THRESHOLD) {
            array_merge_sort(arr, left, right);
        } else {
            // Parallel tasks for left and right halves if big enough
            #pragma omp task if(mid - left > THRESHOLD)
            array_merge_sort_parallel_task(arr, left, mid);

            #pragma omp task if(right - (mid + 1) > THRESHOLD)
            array_merge_sort_parallel_task(arr, mid + 1, right);

            #pragma omp taskwait

            // Merge
            int n1 = mid - left + 1;
            int n2 = right - mid;
            int* L = (int*)malloc(n1 * sizeof(int));
            int* R = (int*)malloc(n2 * sizeof(int));
            for (int i = 0; i < n1; i++) L[i] = arr[left + i];
            for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

            int i = 0, j = 0, k = left;
            while (i < n1 && j < n2) arr[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
            while (i < n1) arr[k++] = L[i++];
            while (j < n2) arr[k++] = R[j++];

            free(L);
            free(R);
        }
    }
}

void array_merge_sort_parallel(int* array, int N, int num_threads) {
    omp_set_num_threads(num_threads);
    #pragma omp parallel
    {
        #pragma omp single
        array_merge_sort_parallel_task(array, 0, N - 1);
    }
}
