import subprocess
import re
import matplotlib.pyplot as plt

# Configuration
degrees = [100_000, 1_000_000]
threads_list = [1, 2, 4, 8, 16]
executable = "./polymul"
num_runs = 5

# Regex patterns for sequential and parallel
seq_time_pattern = re.compile(r"Sequential multiplication time:\s*([\d.]+)\s*s")
par_time_pattern = re.compile(r"Parallel multiplication time:\s*([\d.]+)\s*s")

# Results dictionary
results = {
    "sequential": {1: []},
    "parallel": {t: [] for t in threads_list}
}

# Function to get mean time
def get_mean_time(mode, degree, threads=None):
    times = []
    for run in range(num_runs):
        cmd = [executable, "-d", str(degree), "-m", mode, "--time"]
        if threads:
            cmd += ["-t", str(threads)]
        output = subprocess.run(cmd, capture_output=True, text=True).stdout
        
        # Print output for this run
        print(f"Run {run+1}, mode={mode}, degree={degree}, threads={threads if threads else 1}:\n{output}")

        # Choose the correct regex
        pattern = par_time_pattern if mode == "parallel" else seq_time_pattern
        match = pattern.search(output)
        if match:
            times.append(float(match.group(1)))

    return sum(times) / len(times) if times else None

# Sequential
for degree in degrees:
    mean_time = get_mean_time("sequential", degree)
    results["sequential"][1].append((degree, mean_time))

# Parallel
for degree in degrees:
    for threads in threads_list:
        mean_time = get_mean_time("parallel", degree, threads)
        results["parallel"][threads].append((degree, mean_time))


def plot_parallel_efficiency(results, min_serial_time=1e-6, save_path=None):
    """
    Plot speedup and efficiency vs threads, with ideal lines.
    Optionally save the figure to a file.
    """

    # Extract serial times
    serial_times = dict(results['sequential'][1])
    degrees = [d for d, _ in results['sequential'][1]]
    threads_list = sorted(results['parallel'].keys())

    # Filter out problematic degrees (T1=0 or too tiny)
    degrees = [d for d in degrees if serial_times[d] > min_serial_time]

    plt.figure(figsize=(14,5))

    # ========== SPEEDUP PLOT ==========
    plt.subplot(1,2,1)

    for degree in degrees:
        T1 = serial_times[degree]
        speedups = []
        for threads in threads_list:
            Tp = dict(results['parallel'][threads])[degree]
            speedups.append(T1 / Tp)
        plt.plot(threads_list, speedups, marker='o', linewidth=2, label=f"degree={degree}")

    # Ideal line
    plt.plot(threads_list, threads_list, '--', color='black', linewidth=1.5, label="ideal speedup")

    plt.xlabel("Threads")
    plt.ylabel("Speedup")
    plt.title("Speedup vs Threads")
    plt.grid(True, which='both', ls='--', lw=0.5)
    plt.legend()

    # ========== EFFICIENCY PLOT ==========
    plt.subplot(1,2,2)

    for degree in degrees:
        T1 = serial_times[degree]
        efficiencies = []
        for threads in threads_list:
            Tp = dict(results['parallel'][threads])[degree]
            speedup = T1 / Tp
            efficiencies.append(speedup / threads)
        plt.plot(threads_list, efficiencies, marker='o', linewidth=2, label=f"degree={degree}")

    # Ideal efficiency = 1
    plt.axhline(1.0, linestyle='--', color='black', linewidth=1.5, label="ideal efficiency")

    plt.xlabel("Threads")
    plt.ylabel("Efficiency")
    plt.title("Efficiency vs Threads")
    plt.grid(True, which='both', ls='--', lw=0.5)
    plt.legend()

    plt.tight_layout()

    # Save figure if path is given
    if save_path:
        plt.savefig(save_path, dpi=300)
        print(f"Figure saved to {save_path}")

    plt.show()

plot_parallel_efficiency(results, save_path="parallel_efficiency.png")