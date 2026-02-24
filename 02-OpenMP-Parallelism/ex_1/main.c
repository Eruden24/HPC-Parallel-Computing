#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "polynomial.h"

const char *mannual = "Usage: ./polymul --degree <degree> [--mode <mode>] [--threads <num_threads>] [--time] [--verify] [--help]\n"
                      "Options:\n"
                      "  --degree, -d       Degree of the polynomials (required)\n"
                      "  --mode, -m         Mode of multiplication: 'sequential' or 'parallel' (default: sequential)\n"
                      "  --threads, -t      Number of threads to use in parallel mode (default: 1)\n"
                      "  --time             Output timing information\n"
                      "  --verify, -v       Verify correctness of parallel multiplication\n"
                      "  --help, -h         Display this help message\n";

double get_elapsed_time(struct timespec start, struct timespec end) {
    return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
}

int main(int argc, char *argv[]) {
    int degree = -1, threads = 1, parallel = 0, timing = 0, verify = 0;

    int opt, option_index = 0;
    static struct option long_options[] = {
        {"degree", required_argument, 0, 'd'},
        {"mode", required_argument, 0, 'm'},
        {"threads", required_argument, 0, 't'},
        {"time", no_argument, 0, 0},
        {"verify", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0,0,0,0}
    };

    while ((opt = getopt_long(argc, argv, "d:m:t:vh", long_options, &option_index)) != -1) {
        switch(opt) {
            case 'd': degree = atoi(optarg); break;
            case 'm':
                if (strcmp(optarg, "sequential") == 0) parallel = 0;
                else if (strcmp(optarg, "parallel") == 0) parallel = 1;
                else { fprintf(stderr, "Unknown mode\n"); return 1; }
                break;
            case 't': threads = atoi(optarg); break;
            case 'v': verify = 1; break;
            case 'h': printf("%s", mannual); return 0;
            case 0:
                if (strcmp(long_options[option_index].name, "time") == 0) timing = 1;
                break;
            default: printf("%s", mannual); return 1;
        }
    }

    if (degree < 0) { fprintf(stderr,"Degree required\n"); return 1; }
    if (parallel && threads < 1) { fprintf(stderr,"Threads >= 1 required\n"); return 1; }

    struct timespec gen_start, gen_end, comp_start, comp_end;
    clock_gettime(CLOCK_MONOTONIC, &gen_start);
    srand(time(NULL));
    polynomial p1 = polynomial_random(degree);
    polynomial p2 = polynomial_random(degree);
    clock_gettime(CLOCK_MONOTONIC, &gen_end);
    printf("Polynomial generation time: %.6f s\n", get_elapsed_time(gen_start, gen_end));

    polynomial p3 = NULL;

    clock_gettime(CLOCK_MONOTONIC, &comp_start);
    if (!parallel) {
        p3 = polynomial_naive_sequential(p1, p2, degree);
        clock_gettime(CLOCK_MONOTONIC, &comp_end);
        if (timing) {
            printf("Sequential multiplication time: %.6f s\n", get_elapsed_time(comp_start, comp_end));
        }
    } else {
        p3 = polynomial_naive_parallel(p1, p2, degree, threads);
        clock_gettime(CLOCK_MONOTONIC, &comp_end);
        if (timing) {
            printf("Parallel multiplication time: %.6f s\n", get_elapsed_time(comp_start, comp_end));
        }
    }

    if (!timing) {
        printf("Polynomial 1:\n");
        polynomial_print(p1, degree);
        printf("Polynomial 2:\n");
        polynomial_print(p2, degree);
        printf("Resultant Polynomial:\n");
        polynomial_print(p3, 2*degree);
    }

    if (verify && !parallel) {
        // Sequential naive is always correct; nothing extra needed
        printf("Verification successful: Multiplication is correct.\n");
    } else if (verify && parallel) {
        polynomial p4 = polynomial_naive_sequential(p1, p2, degree);
        if (polynomial_compare(p3, p4, degree))
            printf("Verification successful: Multiplication is correct.\n");
        else
            printf("Verification failed: Multiplication is incorrect.\n");
        polynomial_destroy(p4);
    }

    polynomial_destroy(p1);
    polynomial_destroy(p2);
    polynomial_destroy(p3);

    return 0;
}
