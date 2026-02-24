import subprocess
import matplotlib.pyplot as plt
import os

# -----------------------------
# ΡΥΘΜΙΣΕΙΣ BENCHMARK
# -----------------------------
thread_values = [1, 2, 4, 8, 16]                 # διαφορετικά threads
iteration_values = [10, 100, 200, 500, 1000, 10000, 100000]    # πολύ μικρά + μεγαλύτερα iterations

methods = {
    0: "Mutex",
    1: "Atomic",
    2: "RWLock"
}

executable = "./main"   # το compiled C πρόγραμμα
if not os.path.isfile(executable):
    raise FileNotFoundError(f"Cannot find '{executable}'. Compile your C code first!")

# Αποθήκευση αποτελεσμάτων
results = {m: {} for m in methods}

# -----------------------------
# Συναρτήσεις
# -----------------------------
def run_program(threads, iterations, method):
    """Τρέχει το C πρόγραμμα και επιστρέφει τον χρόνο εκτέλεσης σε δευτερόλεπτα"""
    cmd = [executable, str(threads), str(iterations), str(method)]
    output = subprocess.check_output(cmd).decode()
    for line in output.split("\n"):
        if "Execution time:" in line:
            return float(line.split(":")[1].strip().split()[0])
    raise ValueError("Failed to parse execution time")

# -----------------------------
# Εκτέλεση πειραμάτων
# -----------------------------
print("➡️ Running benchmarks...")

num_runs = 5  # μπορείς να το αυξήσεις σε 10 αν θέλεις

for method in methods:
    for threads in thread_values:
        for iterations in iteration_values:
            times = []
            for run in range(num_runs):
                print(f"Running {methods[method]}: Threads={threads}, Iterations={iterations} (Run {run+1}/{num_runs})")
                t = run_program(threads, iterations, method)
                times.append(t)
            avg_time = sum(times) / num_runs
            results[method][(threads, iterations)] = avg_time
            print(f"✅ Average time: {avg_time:.6f} sec")

# -----------------------------
# Δημιουργία ξεχωριστού plot για κάθε iteration
# -----------------------------
markers = ['o', 's', '^']  # διαφορετικά markers για κάθε method

for iterations in iteration_values:
    plt.figure(figsize=(8,5))
    for i, method in methods.items():
        y = [results[i][(t, iterations)] for t in thread_values]
        plt.plot(thread_values, y, marker=markers[i], linestyle='-', label=method)
    
    plt.xlabel("Threads")
    plt.ylabel("Execution Time (sec)")
    plt.title(f"Locking Methods Comparison (Iterations={iterations})")
    plt.grid(True)
    plt.legend()
    plt.tight_layout()
    filename = f"methods_vs_threads_iter{iterations}.png"
    plt.savefig(filename)
    plt.show()
    print(f"✔ Plot saved: {filename}")

print("✅ All plots generated successfully!")
