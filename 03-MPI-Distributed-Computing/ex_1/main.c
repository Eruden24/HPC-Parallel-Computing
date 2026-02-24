#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <mpi.h>
#include "polynomial.h"

const char *manual =
    "Usage: mpiexec -n <P> ./polymul --degree <n> [--time] [--verify]\n";

static inline double elapsed(struct timespec s, struct timespec e)
{
    return (e.tv_sec - s.tv_sec) + (e.tv_nsec - s.tv_nsec) / 1e9;
}

polynomial polynomial_naive_mpi(polynomial a, polynomial b, int n, int rank, int P, double *recv_time) {

    int result_size = 2 * n + 1;
    int my_len = (result_size - rank + P - 1) / P;
    polynomial local_res = calloc(my_len, sizeof(long long));

    // compute local_res …
    int idx = 0;
    for (int k = rank; k < result_size; k += P) {
        long long sum = 0;
        int i_start = (k > n) ? k - n : 0;
        int i_end   = (k < n) ? k : n;
        for (int i = i_start; i <= i_end; i++)
            sum += a[i] * b[k - i];
        local_res[idx++] = sum;
    }

    // ========= Gatherv timing =========
    struct timespec t_s, t_e;
    if (rank == 0) clock_gettime(CLOCK_MONOTONIC, &t_s);

    int *recvcounts = NULL, *displs = NULL;
    polynomial gathered = NULL;
    if (rank == 0) recvcounts = malloc(P * sizeof(int));

    MPI_Gather(&my_len, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        displs = malloc(P * sizeof(int));
        int total = 0;
        for (int r = 0; r < P; r++) { displs[r] = total; total += recvcounts[r]; }
        gathered = malloc(total * sizeof(long long));
    }

    MPI_Gatherv(local_res, my_len, MPI_LONG_LONG,gathered, recvcounts, displs, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        clock_gettime(CLOCK_MONOTONIC, &t_e);
        if (recv_time) *recv_time = elapsed(t_s, t_e);
    }

    free(local_res);

    // reconstruct full result
    if (rank == 0) {
        polynomial result = calloc(result_size, sizeof(long long));
        int *pos = calloc(P, sizeof(int));
        for (int k = 0; k < result_size; k++) {
            int r = k % P;
            result[k] = gathered[displs[r] + pos[r]];
            pos[r]++;
        }
        free(pos); free(gathered); free(recvcounts); free(displs);
        return result;
    }
    return NULL;
}


/* ==================== Main ==================== */
int main(int argc, char *argv[])
{
    int n = -1, timing = 0, verify = 0;

    int opt, idx = 0;
    static struct option long_options[] = {
        {"degree", required_argument, 0, 'd'},
        {"time",   no_argument,       0, 't'},
        {"verify", no_argument,       0, 'v'},
        {"help",   no_argument,       0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "d:tvh",
                              long_options, &idx)) != -1) {
        switch (opt) {
        case 'd': n = atoi(optarg); break;
        case 't': timing = 1; break;
        case 'v': verify = 1; break;
        case 'h': printf("%s", manual); return 0;
        default:  printf("%s", manual); return 1;
        }
    }

    if (n < 0) {
        fprintf(stderr, "Degree required. %s", manual);
        return 1;
    }

    MPI_Init(&argc, &argv);
    int rank, P;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &P);

    polynomial a = malloc((n + 1) * sizeof(long long));
    polynomial b = malloc((n + 1) * sizeof(long long));

    if (rank == 0) {
        srand(42);
        polynomial ta = polynomial_random(n);
        polynomial tb = polynomial_random(n);
        memcpy(a, ta, (n + 1) * sizeof(long long));
        memcpy(b, tb, (n + 1) * sizeof(long long));
        free(ta);
        free(tb);
    }

    /* =========================
       Timing variables
       ========================= */
    struct timespec total_s, total_e;
    struct timespec send_s, send_e;
    struct timespec comp_s, comp_e;

    // MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) clock_gettime(CLOCK_MONOTONIC, &total_s);

    /* =========================
       (i) Data send (Broadcast)
       ========================= */
    // MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) clock_gettime(CLOCK_MONOTONIC, &send_s);

    MPI_Bcast(a, n + 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(b, n + 1, MPI_LONG_LONG, 0, MPI_COMM_WORLD);

    if (rank == 0) clock_gettime(CLOCK_MONOTONIC, &send_e);
    if (rank == 0 && timing)
        printf("(i) Data send time: %.6f s\n", elapsed(send_s, send_e));

    /* =========================
       (ii) Parallel compute
       ========================= */
    // MPI_Barrier(MPI_COMM_WORLD);
    clock_gettime(CLOCK_MONOTONIC, &comp_s);

    double recv_time = 0;
    polynomial result = polynomial_naive_mpi(a, b, n, rank, P, &recv_time);

    clock_gettime(CLOCK_MONOTONIC, &comp_e);
    double local_comp = elapsed(comp_s, comp_e);
    double max_comp;
    MPI_Reduce(&local_comp, &max_comp, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0 && timing)
        printf("(ii) Parallel compute time: %.6f s\n", max_comp);

    /* =========================
       (iii) Result receive
       ========================= */
    if (rank == 0 && timing)
        printf("(iii) Result receive time: %.6f s\n", recv_time);

    /* =========================
       (iv) Total execution
       ========================= */
    if (rank == 0) {
        clock_gettime(CLOCK_MONOTONIC, &total_e);
        printf("(iv) Total execution time: %.6f s\n", elapsed(total_s, total_e));

        /* =========================
           Verification / output
           ========================= */
        if (verify) {
            printf("Verifying result...\n");
            polynomial seq = polynomial_naive_sequential(a, b, n);
            if (polynomial_compare(result, seq, n))
                printf("SUCCESS: Results match.\n");
            else
                printf("FAILURE: Results differ!\n");
            free(seq);
        } else if (!timing && n < 20) {
            polynomial_print(result, 2 * n);
        }

        free(result);
    }

    free(a);
    free(b);
    MPI_Finalize();
    
    return 0;
}
