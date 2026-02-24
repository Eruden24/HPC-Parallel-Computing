import subprocess
import matplotlib.pyplot as plt

# --- PARAMETERS ---
array_sizes = [10_000_000, 100_000_000]  # 10^7 and 10^8
threads_list = [1, 2, 4, 8, 16]
program = "./main"
output_file = "speedup_efficiency.png"
num_runs = 3  # Number of runs to average

# --- COLORS for plotting ---
colors = ['blue', 'orange']

# --- Initialize data storage ---
speedup_data = {}
efficiency_data = {}

def get_average_time(args):
    """Run the program num_runs times and return the average elapsed time."""
    times = []
    for _ in range(num_runs):
        result = subprocess.run(args, capture_output=True, text=True)
        time_taken = None
        for line in result.stdout.splitlines():
            if line.startswith("Time elapsed"):
                time_taken = float(line.split(":")[1].strip().split()[0])
                break
        if time_taken is None:
            raise RuntimeError(f"Failed to get time from output:\n{result.stdout}")
        times.append(time_taken)
    return sum(times) / len(times)

for size in array_sizes:
    print(f"\n=== Array size: {size} ===")
    
    # --- Run serial ---
    print("Running serial version...")
    serial_time = get_average_time([program, str(size), "serial"])
    print(f"Average serial time over {num_runs} runs: {serial_time:.4f}s")
    
    # --- Run parallel versions ---
    parallel_times = []
    for t in threads_list:
        print(f"Running parallel with {t} threads...")
        avg_time = get_average_time([program, str(size), "parallel", str(t)])
        parallel_times.append(avg_time)
        print(f"Average time: {avg_time:.4f}s")
    
    # --- Compute speedup and efficiency ---
    speedup = [serial_time / pt for pt in parallel_times]
    efficiency = [s / t for s, t in zip(speedup, threads_list)]
    
    speedup_data[size] = speedup
    efficiency_data[size] = efficiency

# --- PLOT RESULTS ---
plt.figure(figsize=(12,5))

# Speedup plot
plt.subplot(1,2,1)
for i, size in enumerate(array_sizes):
    plt.plot(threads_list, speedup_data[size], 'o-', color=colors[i], label=f"{size} elements")
plt.plot(threads_list, threads_list, 'r--', label="Ideal speedup")
plt.xlabel("Number of Threads")
plt.ylabel("Speedup")
plt.title("Speedup vs Threads")
plt.grid(True)
plt.legend()

# Efficiency plot
plt.subplot(1,2,2)
for i, size in enumerate(array_sizes):
    plt.plot(threads_list, efficiency_data[size], 'o-', color=colors[i], label=f"{size} elements")
plt.plot(threads_list, [1]*len(threads_list), 'r--', label="Ideal efficiency")
plt.xlabel("Number of Threads")
plt.ylabel("Efficiency")
plt.title("Efficiency vs Threads")
plt.grid(True)
plt.legend()

plt.tight_layout()

# --- SAVE FIGURE ---
plt.savefig(output_file, dpi=300)
print(f"Plot saved as {output_file}")

# --- SHOW FIGURE ---
plt.show()
