#ifndef ARRAY_H
#define ARRAY_H

// Allocate an integer array of size N
int* array_allocate(int N);

// Free an integer array
void array_free(int* array);

// Initialize array with deterministic random integers [0, 99]
void array_init_random(int* array, int N);

// Print the array to stdout
void array_print(int* array, int N);

// Check if the array is sorted in ascending order
int array_is_sorted(int* array, int N);

// Serial mergesort: sort arr[left..right]
void array_merge_sort(int arr[], int left, int right);

// Parallel mergesort using OpenMP tasks
// num_threads: number of OpenMP threads to use
void array_merge_sort_parallel(int* array, int N, int num_threads);

#endif // ARRAY_H
