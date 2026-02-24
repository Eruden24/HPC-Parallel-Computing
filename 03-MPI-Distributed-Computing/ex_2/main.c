#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include "utils.h"

int main(int argc, char *argv[]) {
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 4) {
        if (rank == 0) printf("Usage: %s <N> <sparsity> <iterations>\n", argv[0]);
        MPI_Finalize();
        return 0;
    }

    int N = atoi(argv[1]);
    double sparsity = atof(argv[2]);
    int iterations = atoi(argv[3]);

    // if (N % size != 0) {
    //     if (rank == 0) printf("Error: N must be divisible by number of processes.\n");
    //     MPI_Finalize();
    //     return 0;
    // }

    int local_N = N / size;

    /* --- Variables Declaration --- */
    int *global_dense = NULL;
    int *global_vector = NULL;
    CSR *global_csr = NULL;
    
    int *local_vector_x = (int*)malloc(N * sizeof(int)); // Stores the FULL vector x
    int *local_result_y = (int*)malloc(local_N * sizeof(int)); // Stores partial y
    
    /* --- Phase 0: Initialization (Rank 0 only) --- */
    if (rank == 0) {
        srand(time(NULL));
        global_dense = generate_dense_matrix(N, sparsity);
        global_vector = generate_vector(N);
        
        // Initial copy for first iteration
        memcpy(local_vector_x, global_vector, N * sizeof(int));
    }

    // Broadcast initial vector to all (so everyone starts with same x)
    MPI_Bcast(local_vector_x, N, MPI_INT, 0, MPI_COMM_WORLD);

    // Variables for timers
// Άλλαξε το:
// double t_construct_start, t_construct_end, t_csr_build = 0.0;
// ...

// Σε:
	double t_construct_start = 0.0, t_construct_end = 0.0, t_csr_build = 0.0;
	double t_comm_start = 0.0, t_comm_end = 0.0, t_csr_comm = 0.0;
	double t_calc_start = 0.0, t_calc_end = 0.0, t_csr_calc = 0.0;
    double t_dense_total = 0.0;


    /* ======================================================
       PART A: CSR Implementation
       ====================================================== */

    /* 1. CSR Construction (Rank 0) */
    if (rank == 0) {
        t_construct_start = MPI_Wtime();
        global_csr = dense_to_csr(global_dense, N);
        t_construct_end = MPI_Wtime();
        t_csr_build = t_construct_end - t_construct_start;
    }

    /* 2. CSR Data Distribution */
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) t_comm_start = MPI_Wtime();

    // Step 2a: Distribute Row structure
    // Instead of sending row_ptr directly, we calculate row_nnz (count per row)
    // and scatter that. Each proc rebuilds its local row_ptr.
    int *global_row_nnz = NULL;
    int *local_row_nnz = (int*)malloc(local_N * sizeof(int));

    if (rank == 0) {
        global_row_nnz = (int*)malloc(N * sizeof(int));
        for(int i=0; i<N; i++) {
            global_row_nnz[i] = global_csr->row_ptr[i+1] - global_csr->row_ptr[i];
        }
    }

    MPI_Scatter(global_row_nnz, local_N, MPI_INT, 
                local_row_nnz, local_N, MPI_INT, 0, MPI_COMM_WORLD);

    // Build local CSR struct
    CSR local_csr;
    local_csr.num_rows = local_N;
    local_csr.row_ptr = (int*)malloc((local_N + 1) * sizeof(int));
    local_csr.row_ptr[0] = 0;
    local_csr.nnz = 0;
    
    for (int i=0; i<local_N; i++) {
        local_csr.nnz += local_row_nnz[i];
        local_csr.row_ptr[i+1] = local_csr.nnz;
    }

    local_csr.values = (int*)malloc(local_csr.nnz * sizeof(int));
    local_csr.col_ind = (int*)malloc(local_csr.nnz * sizeof(int));

    // Step 2b: Distribute Values and Column Indices using Scatterv
    int *sendcounts = NULL;
    int *displs = NULL;

    if (rank == 0) {
        sendcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        
        // Calculate how many NNZ each process gets
        int current_disp = 0;
        for (int i=0; i<size; i++) {
            // Start row for rank i: i*local_N
            // End row for rank i: (i+1)*local_N
            int start_row = i * local_N;
            int end_row = (i + 1) * local_N;
            int count = global_csr->row_ptr[end_row] - global_csr->row_ptr[start_row];
            
            sendcounts[i] = count;
            displs[i] = current_disp;
            current_disp += count;
        }
    }

    MPI_Scatterv(rank == 0 ? global_csr->values : NULL, sendcounts, displs, MPI_INT,
                 local_csr.values, local_csr.nnz, MPI_INT, 
                 0, MPI_COMM_WORLD);

    MPI_Scatterv(rank == 0 ? global_csr->col_ind : NULL, sendcounts, displs, MPI_INT,
                 local_csr.col_ind, local_csr.nnz, MPI_INT, 
                 0, MPI_COMM_WORLD);

    if (rank == 0) {
        t_comm_end = MPI_Wtime();
        t_csr_comm = t_comm_end - t_comm_start;
        free(global_row_nnz);
        free(sendcounts);
        free(displs);
    }
    free(local_row_nnz);

    /* 3. CSR Parallel Execution */
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) t_calc_start = MPI_Wtime();

    // Need a temp buffer for iterations
    for (int iter = 0; iter < iterations; iter++) {
        // Compute part of y
        csr_mat_vec_mult(&local_csr, local_vector_x, local_result_y, local_N);
        
        // Gather results from all procs to update vector x for next iteration
        // Result of this iter becomes input (x) of next
MPI_Allgather(local_result_y, local_N, MPI_INT, 
              local_vector_x, local_N, MPI_INT, MPI_COMM_WORLD);    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        t_calc_end = MPI_Wtime();
        t_csr_calc = t_calc_end - t_calc_start;
    }

    // Cleanup local CSR
    free(local_csr.values);
    free(local_csr.col_ind);
    free(local_csr.row_ptr);


    /* ======================================================
       PART B: Dense Implementation
       ====================================================== */
    
    // Reset vector X for fair comparison
    if (rank == 0) memcpy(local_vector_x, global_vector, N * sizeof(int));
    MPI_Bcast(local_vector_x, N, MPI_INT, 0, MPI_COMM_WORLD);

    int *local_dense = (int*)malloc(local_N * N * sizeof(int));

    MPI_Barrier(MPI_COMM_WORLD);
    double t_dense_start = MPI_Wtime();

    // 1. Scatter Dense Matrix
    MPI_Scatter(global_dense, local_N * N, MPI_INT, 
                local_dense, local_N * N, MPI_INT, 
                0, MPI_COMM_WORLD);

    // 2. Loop
    for (int iter = 0; iter < iterations; iter++) {
        dense_mat_vec_mult(local_dense, local_vector_x, local_result_y, local_N, N);
		MPI_Allgather(local_result_y, local_N, MPI_INT, 
              local_vector_x, local_N, MPI_INT, MPI_COMM_WORLD);
	}

    MPI_Barrier(MPI_COMM_WORLD);
    double t_dense_end = MPI_Wtime();
    if (rank == 0) t_dense_total = t_dense_end - t_dense_start;

    free(local_dense);

    /* ======================================================
       OUTPUT RESULTS
       ====================================================== */
    if (rank == 0) {
        printf("\n--- MPI Results (N=%d, Sparsity=%.2f, Procs=%d, Iters=%d) ---\n", 
               N, sparsity, size, iterations);
        printf("(i)   CSR Construction Time    : %.6f s\n", t_csr_build);
        printf("(ii)  CSR Communication Time   : %.6f s\n", t_csr_comm);
        printf("(iii) CSR Calculation Time     : %.6f s\n", t_csr_calc);
        printf("(iv)  Total CSR Time           : %.6f s\n", t_csr_build + t_csr_comm + t_csr_calc);
        printf("(v)   Total Dense Time         : %.6f s\n", t_dense_total);
        
        // Clean up Rank 0 memory
        free(global_dense);
        free(global_vector);
        free_csr(global_csr);
    }

    free(local_vector_x);
    free(local_result_y);

    MPI_Finalize();
    return 0;
}