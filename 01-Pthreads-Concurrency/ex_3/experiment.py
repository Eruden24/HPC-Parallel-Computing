import subprocess
import re
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np

# -------------------------------
# Plot settings
# -------------------------------
sns.set(style="whitegrid")
plt.rcParams.update({'font.size': 12})

# -------------------------------
# C executable
# -------------------------------
exe = "./main"

# -------------------------------
# Array sizes to test
# -------------------------------
array_sizes = [10**5, 2*10**5, 5*10**5, 10**6, 5*10**6]

# -------------------------------
# Store times
# -------------------------------
seq_times_all = {0: [], 1: []}
par_times_all = {0: [], 1: []}

num_runs = 5  # Repetitions per size

# -------------------------------
# Run benchmark for padding=0 and 1
# -------------------------------
for padding_flag in [0, 1]:
    seq_times_avg = []
    par_times_avg = []

    for size in array_sizes:
        seq_runs = []
        par_runs = []

        print(f"Running array size {size} with padding={padding_flag}...")
        for _ in range(num_runs):
            result = subprocess.run([exe, str(size), str(padding_flag)],
                                    capture_output=True, text=True)
            output = result.stdout

            # Extract sequential and parallel times
            seq_match = re.search(r"Sequential execution time: ([\d.]+)", output)
            par_match = re.search(r"Parallel execution time: ([\d.]+)", output)

            if seq_match and par_match:
                seq_runs.append(float(seq_match.group(1)))
                par_runs.append(float(par_match.group(1)))
            else:
                print(f"Could not parse output for size {size}:\n{output}")

        # Average times for this size
        seq_times_avg.append(np.mean(seq_runs))
        par_times_avg.append(np.mean(par_runs))

    seq_times_all[padding_flag] = np.array(seq_times_avg, dtype=float)
    par_times_all[padding_flag] = np.array(par_times_avg, dtype=float)

# -------------------------------
# Compute speedup and efficiency
# -------------------------------
num_threads = 4
speedup_all = {pf: seq_times_all[pf] / par_times_all[pf] for pf in [0, 1]}

# -------------------------------
# Plot execution times (both padding)
# -------------------------------
plt.figure(figsize=(8,5))
plt.plot(array_sizes, seq_times_all[0], marker='o', label='Sequential (plain)')
plt.plot(array_sizes, par_times_all[0], marker='o', label='Parallel (plain)')
plt.plot(array_sizes, seq_times_all[1], marker='s', label='Sequential (padded)')
plt.plot(array_sizes, par_times_all[1], marker='s', label='Parallel (padded)')
plt.xscale('log')
plt.yscale('log')
plt.xlabel('Array Size')
plt.ylabel('Time (seconds)')
plt.title('Execution Time: Plain vs Padded')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("execution_times_comparison.png")
plt.show()

# -------------------------------
# Plot speedup (both padding)
# -------------------------------
plt.figure(figsize=(8,5))
plt.plot(array_sizes, speedup_all[0], marker='o', label='Speedup (plain)')
plt.plot(array_sizes, speedup_all[1], marker='s', label='Speedup (padded)')
plt.axhline(y=num_threads, color='red', linestyle='--', label=f'Ideal speedup (n={num_threads})')
plt.xscale('log')
plt.xlabel('Array Size')
plt.ylabel('Speedup')
plt.title('Parallel Speedup: Plain vs Padded')
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("speedup_comparison.png")
plt.show()
