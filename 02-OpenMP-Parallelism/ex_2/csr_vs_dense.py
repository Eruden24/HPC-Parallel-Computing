import subprocess
import re
import pandas as pd
import matplotlib.pyplot as plt

# ---------------- PARAMETERS ----------------
Ns = [1000, 10000]
sparsities = [0.0, 0.5, 0.9, 0.99]
Ms = [1, 5, 10, 20]
runs = 3

def get_time(output, label):
    m = re.search(rf"{label}\s*:\s*([\d.]+)", output)
    return float(m.group(1)) if m else None

records = []

# ---------------- BENCHMARK -----------------
for N in Ns:
    for s in sparsities:
        for M in Ms:
            csr_build_times = []
            csr_mult_times = []
            dense_mult_times = []

            for r in range(runs):
                o = subprocess.run(["./main", str(N), str(s), "parallel", "4", str(M)],
                                   capture_output=True, text=True).stdout

                csr_build_times.append(get_time(o, "CSR construction time"))
                csr_mult_times.append(get_time(o, "CSR mat-vec total time"))
                dense_mult_times.append(get_time(o, "Dense mat-vec total time"))

            records.append({
                "N": N,
                "Sparsity": s,
                "M": M,
                "CSR_Build": sum(csr_build_times)/runs,
                "CSR_Mult": sum(csr_mult_times)/runs,
                "Dense_Mult": sum(dense_mult_times)/runs,
            })

df = pd.DataFrame(records)
df["CSR_Total"] = df["CSR_Build"] + df["CSR_Mult"]
df["Speedup_CSR_vs_Dense"] = df["Dense_Mult"] / df["CSR_Total"]

df.to_csv("csr_vs_dense_results.csv", index=False)
print("Results saved to csr_vs_dense_results.csv")

Ns = df["N"].unique()
Ms = df["M"].unique()

colors = ["blue", "green", "orange", "red"]

for N in Ns:
    dfN = df[df["N"] == N]

    # --- Total Time vs Sparsity ---
    plt.figure(figsize=(8,6))
    for i, M in enumerate(Ms):
        dfM = dfN[dfN["M"] == M]
        plt.plot(dfM["Sparsity"], dfM["CSR_Total"], marker='o', linestyle='-', color=colors[i], label=f"CSR Total M={M}")
        plt.plot(dfM["Sparsity"], dfM["Dense_Mult"], marker='x', linestyle='--', color=colors[i], label=f"Dense Mult M={M}")
    plt.xlabel("Sparsity")
    plt.ylabel("Total Time (s)")
    plt.title(f"CSR vs Dense Total Time (N={N})")
    plt.legend()
    plt.grid(True)
    plt.savefig(f"csr_vs_dense_time_N{N}.png")
    plt.show()

    # --- Speedup vs Sparsity ---
    plt.figure(figsize=(8,6))
    for i, M in enumerate(Ms):
        dfM = dfN[dfN["M"] == M]
        speedup = dfM["Dense_Mult"] / dfM["CSR_Total"]
        plt.plot(dfM["Sparsity"], speedup, marker='o', linestyle='-', color=colors[i], label=f"M={M}")
    plt.xlabel("Sparsity")
    plt.ylabel("Speedup (Dense / CSR Total)")
    plt.title(f"CSR vs Dense Speedup (N={N})")
    plt.legend()
    plt.grid(True)
    plt.savefig(f"csr_vs_dense_speedup_N{N}.png")
    plt.show()
