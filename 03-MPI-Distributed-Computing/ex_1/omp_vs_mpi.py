import subprocess
import re
import csv
import matplotlib.pyplot as plt

DEGREES = [10**5, 10**6]
WORKERS = [1, 2, 4, 8, 16]

OMP_EXEC = "./../../assignment_2/ex_1/polymul"
MPI_EXEC = "./polymul"

CSV_FILE = "omp_vs_mpi_results.csv"

def run_and_capture(cmd):
    result = subprocess.run(
        cmd,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    return result.stdout

results = []

# ================= RUN & STORE MEASUREMENTS =================
for DEGREE in DEGREES:
    omp_times = {}
    mpi_times = {}

    # ---- OpenMP ----
    for t in WORKERS:
        output = run_and_capture([
            OMP_EXEC,
            "--degree", str(DEGREE),
            "--mode", "parallel",
            "--threads", str(t),
            "--time"
        ])

        match = re.search(r"Parallel multiplication time:\s+([0-9.]+)\s+s", output)
        if not match:
            raise RuntimeError(f"OpenMP time not found (degree={DEGREE}, threads={t})")

        omp_times[t] = float(match.group(1))

    # ---- MPI ----
    for p in WORKERS:
        output = run_and_capture([
            "mpiexec",
            "--oversubscribe",
            "-n", str(p),
            MPI_EXEC,
            "--degree", str(DEGREE),
            "--time"
        ])

        match = re.search(r"\(ii\)\s+Parallel compute time:\s+([0-9.]+)\s+s", output)
        if not match:
            raise RuntimeError(f"MPI time not found (degree={DEGREE}, procs={p})")

        mpi_times[p] = float(match.group(1))

    # ---- Store results ----
    for w in WORKERS:
        results.append([
            DEGREE,
            w,
            omp_times[w],
            mpi_times[w]
        ])

# ================= SAVE TO CSV =================
with open(CSV_FILE, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow([
        "degree",
        "workers",
        "omp_time",
        "mpi_time"
    ])
    writer.writerows(results)

print(f"Results saved to {CSV_FILE}")

# ================= LOAD & PLOT =================
for DEGREE in DEGREES:
    workers = []
    omp_times = []
    mpi_times = []

    for row in results:
        if row[0] == DEGREE:
            workers.append(row[1])
            omp_times.append(row[2])
            mpi_times.append(row[3])

    # ---- Speedup ----
    omp_speedup = [omp_times[0] / t for t in omp_times]
    mpi_speedup = [mpi_times[0] / t for t in mpi_times]

    # ---- Efficiency ----
    omp_eff = [s / p for s, p in zip(omp_speedup, workers)]
    mpi_eff = [s / p for s, p in zip(mpi_speedup, workers)]

    # ========== FIGURE 1: Execution Time ==========
    plt.figure()
    plt.plot(workers, omp_times, marker="o", label="OpenMP")
    plt.plot(workers, mpi_times, marker="s", label="MPI")
    plt.xlabel("Threads / Processes")
    plt.ylabel("Execution Time (s)")
    plt.title(f"Execution Time (Degree {DEGREE})")
    plt.legend()
    plt.grid(True)

    # ========== FIGURE 2: Speedup ==========
    plt.figure()
    plt.plot(workers, omp_speedup, marker="o", label="OpenMP")
    plt.plot(workers, mpi_speedup, marker="s", label="MPI")
    plt.xlabel("Threads / Processes")
    plt.ylabel("Speedup")
    plt.title(f"Speedup (Degree {DEGREE})")
    plt.legend()
    plt.grid(True)

    # ========== FIGURE 3: Efficiency ==========
    plt.figure()
    plt.plot(workers, omp_eff, marker="o", label="OpenMP")
    plt.plot(workers, mpi_eff, marker="s", label="MPI")
    plt.xlabel("Threads / Processes")
    plt.ylabel("Efficiency")
    plt.title(f"Efficiency (Degree {DEGREE})")
    plt.legend()
    plt.grid(True)

plt.show()
