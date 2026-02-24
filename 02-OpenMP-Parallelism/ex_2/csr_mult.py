import subprocess
import re
import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns

# ------------------ PARAMETERS ------------------
Ns = [1000, 10000]                   # Matrix sizes
sparsities = [0.0, 0.5, 0.9, 0.99]  # Fraction of zeros
threads_list = [1, 2, 4, 8, 16]
runs = 3

def get_time(output, label):
    """Extract time value from program output."""
    m = re.search(rf"{label}\s*:\s*([\d.]+)", output)
    return float(m.group(1)) if m else None

records = []

# ------------------ CSR MULTIPLICATION BENCHMARK ------------------
for N in Ns:
    for sparsity in sparsities:
        print(f"\nCSR Multiplication benchmark for N={N}, sparsity={sparsity}")

        # --- Serial CSR Multiplication ---
        ser_mult_times = []
        for r in range(runs):
            o = subprocess.run(["./main", str(N), str(sparsity), "serial", "1"],
                               capture_output=True, text=True).stdout
            ser_mult_times.append(get_time(o, "CSR mat-vec total time"))
        ser_mult_avg = sum(ser_mult_times) / runs

        # --- Parallel CSR Multiplication ---
        for t in threads_list:
            par_mult_times = []
            for r in range(runs):
                o = subprocess.run(["./main", str(N), str(sparsity), "parallel", str(t), "1"],
                                   capture_output=True, text=True).stdout
                par_mult_times.append(get_time(o, "CSR mat-vec total time"))
            par_mult_avg = sum(par_mult_times) / runs

            records.append({"N": N, "Sparsity": sparsity, "Threads": t,
                            "Serial": ser_mult_avg, "Parallel": par_mult_avg})

# ------------------ SAVE RESULTS ------------------
df = pd.DataFrame(records)
df.to_csv("csr_mult_results_sparsity.csv", index=False)
print("Results saved to csr_mult_results_sparsity.csv")

# ------------------ PLOTTING ------------------
colors = {0.0:'blue', 0.5:'green', 0.9:'orange', 0.99:'red'}

for N in Ns:
    # --- Time vs Threads ---
    plt.figure(figsize=(8,6))
    for s in sparsities:
        mult = df[(df["N"] == N) & (df["Sparsity"] == s)]
        plt.plot(mult["Threads"], mult["Parallel"], marker='o', linestyle='-', color=colors[s], label=f"Sparsity={s}")
        plt.hlines(mult["Serial"].iloc[0], xmin=min(threads_list), xmax=max(threads_list),
                   colors=colors[s], linestyles='--')
    plt.xlabel("Threads")
    plt.ylabel("CSR mat-vec Time (s)")
    plt.title(f"CSR Multiplication Time vs Threads (N={N})")
    plt.legend()
    plt.grid(True)
    plt.savefig(f"csr_mult_time_N{N}.png")
    plt.show()

    # --- Speedup vs Threads ---
    plt.figure(figsize=(8,6))
    for s in sparsities:
        mult = df[(df["N"] == N) & (df["Sparsity"] == s)]
        speedup = mult["Serial"] / mult["Parallel"]
        plt.plot(mult["Threads"], speedup, marker='o', linestyle='-', color=colors[s], label=f"Sparsity={s}")
    plt.plot(threads_list, threads_list, linestyle=':', color='gray', label="Ideal Speedup")
    plt.xlabel("Threads")
    plt.ylabel("Speedup")
    plt.title(f"CSR Multiplication Speedup vs Threads (N={N})")
    plt.legend()
    plt.grid(True)
    plt.savefig(f"csr_mult_speedup_N{N}.png")
    plt.show()

    # --- Efficiency vs Threads ---
    plt.figure(figsize=(8,6))
    for s in sparsities:
        mult = df[(df["N"] == N) & (df["Sparsity"] == s)]
        speedup = mult["Serial"] / mult["Parallel"]
        efficiency = speedup / mult["Threads"]
        plt.plot(mult["Threads"], efficiency, marker='o', linestyle='-', color=colors[s], label=f"Sparsity={s}")
    plt.axhline(1.0, linestyle=':', color='gray', label="Ideal Efficiency")
    plt.xlabel("Threads")
    plt.ylabel("Efficiency")
    plt.title(f"CSR Multiplication Efficiency vs Threads (N={N})")
    plt.legend()
    plt.grid(True)
    plt.savefig(f"csr_mult_efficiency_N{N}.png")
    plt.show()

# --- Optional: Heatmap of Time vs Threads/Sparsity ---
for N in Ns:
    pivot = df[df["N"] == N].pivot(index="Sparsity", columns="Threads", values="Parallel")
    plt.figure(figsize=(10,6))
    sns.heatmap(pivot, annot=True, fmt=".3f", cmap="YlGnBu")
    plt.title(f"CSR Multiplication Time Heatmap (N={N})")
    plt.xlabel("Threads")
    plt.ylabel("Sparsity")
    plt.savefig(f"csr_mult_heatmap_N{N}.png")
    plt.show()

print("All plots saved.")
