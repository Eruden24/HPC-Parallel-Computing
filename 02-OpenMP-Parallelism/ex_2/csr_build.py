import subprocess
import re
import matplotlib.pyplot as plt
import pandas as pd

# ------------------ PARAMETERS ------------------
Ns = [1000, 10000]          # Matrix sizes
threads_list = [1, 2, 4, 8, 16]
runs = 3
sparsity = 0.95

def get_time(output, label):
    """Extract time value from program output."""
    m = re.search(rf"{label}\s*:\s*([\d.]+)", output)
    return float(m.group(1)) if m else None

records = []

# ------------------ CSR BUILD BENCHMARK ------------------
for N in Ns:
    print(f"\nCSR Build benchmark for N = {N}")

    # Serial CSR Build
    ser_build_times = []
    for r in range(runs):
        o = subprocess.run(["./main", str(N), str(sparsity), "serial", "1"],
                           capture_output=True, text=True).stdout
        ser_build_times.append(get_time(o, "CSR construction time"))
    ser_build_avg = sum(ser_build_times) / runs

    # Parallel CSR Build
    for t in threads_list:
        par_build_times = []
        for r in range(runs):
            o = subprocess.run(["./main", str(N), str(sparsity), "parallel", str(t), "1"],
                               capture_output=True, text=True).stdout
            par_build_times.append(get_time(o, "CSR construction time"))
        par_build_avg = sum(par_build_times) / runs

        records.append({"N": N, "Threads": t,
                        "Serial": ser_build_avg, "Parallel": par_build_avg})

df = pd.DataFrame(records)
df.to_csv("csr_build_results.csv", index=False)
print("Results saved to csr_build_results.csv")

# ------------------ PLOTTING ------------------
colors = {1000:'blue', 10000:'red'}

# --- Build Time vs Threads (both Serial and Parallel) ---
plt.figure(figsize=(8,6))
for N in Ns:
    build = df[df["N"] == N]
    # Parallel times vs threads
    plt.plot(build["Threads"], build["Parallel"], marker='o', linestyle='-', color=colors[N], label=f"Parallel N={N}")
    # Serial time as horizontal line
    plt.hlines(build["Serial"].iloc[0], xmin=min(threads_list), xmax=max(threads_list),
               colors=colors[N], linestyles='--', label=f"Serial N={N}")

plt.xlabel("Threads")
plt.ylabel("CSR Build Time (s)")
plt.title("CSR Build Time vs Threads")
plt.legend()
plt.grid(True)
plt.savefig("csr_build_time.png")
plt.show()


# --- Speedup vs Threads ---
plt.figure(figsize=(8,6))
for N in Ns:
    build = df[df["N"] == N]
    speedup = build["Serial"] / build["Parallel"]
    plt.plot(build["Threads"], speedup, marker='o', linestyle='-', color=colors[N], label=f"N={N}")
# Ideal speedup
ideal_threads = threads_list
plt.plot(ideal_threads, ideal_threads, linestyle=':', color='gray', label="Ideal Speedup")
plt.xlabel("Threads")
plt.ylabel("Speedup")
plt.title("CSR Build Speedup vs Threads")
plt.legend()
plt.grid(True)
plt.savefig("csr_build_speedup.png")
plt.show()

# --- Efficiency vs Threads ---
plt.figure(figsize=(8,6))
for N in Ns:
    build = df[df["N"] == N]
    speedup = build["Serial"] / build["Parallel"]
    efficiency = speedup / build["Threads"]
    plt.plot(build["Threads"], efficiency, marker='o', linestyle='-', color=colors[N], label=f"N={N}")
# Ideal efficiency line
plt.axhline(1.0, linestyle=':', color='gray', label="Ideal Efficiency")
plt.xlabel("Threads")
plt.ylabel("Efficiency")
plt.title("CSR Build Efficiency vs Threads")
plt.legend()
plt.grid(True)
plt.savefig("csr_build_efficiency.png")
plt.show()

print("Plots saved as csr_build_time.png, csr_build_speedup.png, csr_build_efficiency.png")
