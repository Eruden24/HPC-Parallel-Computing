# High-Performance Computing & Parallel Systems (UoA)

This repository contains a series of projects focused on parallel and distributed computing, implemented as part of the **Parallel Systems** curriculum at the National and Kapodistrian University of Athens (2025-2026).

The projects explore different parallel programming models and synchronization primitives using **C** in a Linux cluster environment.

## 📂 Repository Structure

### 1. [Pthreads-Concurrency](./01-Pthreads/)
Focuses on shared-memory parallelism and low-level thread management.
* **Key Implementations:** Polynomial multiplication, concurrent shared variable updates, and custom barrier synchronization.
* **Concepts:** Race conditions, Mutexes, Read-Write Locks, Atomic built-ins, and Sense-reversal barriers.
* **Performance Analysis:** Comparison between different locking granularities (coarse-grained vs. fine-grained).

### 2. [OpenMP-Parallelism](./02-OpenMP/)
Explores high-level directive-based parallelism for shared-memory systems.
* **Key Implementations:** * Efficient **Sparse Matrix-Vector Multiplication (SpMV)** using the **CSR (Compressed Sparse Row)** format.
    * Recursive **Mergesort** using OpenMP `tasks`.
* **Concepts:** Data distribution, load balancing, and performance comparison between CSR and Dense matrix representations.

### 3. [MPI-Distributed-Computing](./03-MPI/)
Focuses on the message-passing model for distributed-memory environments (multi-node clusters).
* **Key Implementations:** Distributed Polynomial Multiplication and Sparse Matrix-Vector Multiplication.
* **Concepts:** Point-to-point communication (`MPI_Send`/`MPI_Recv`), Collective communications (`MPI_Bcast`, `MPI_Gather`), and Communication vs. Computation overhead analysis.

---

## 🛠️ Technical Highlights
* **Language:** C (C11 standard).
* **Optimization:** Used profiling techniques to achieve speedup across multiple cores and nodes.
* **Data Structures:** Custom implementation of the CSR format to optimize memory footprint for sparse data.
* **Environment:** Developed and tested on the University’s Linux Cluster (`linuxXX.di.uoa.gr`).

## 🚀 How to Run
Each project folder contains a `Makefile`. To compile and run: