import subprocess
import csv
import re

# ==========================================
# CONFIGURATION
# ==========================================
# 1. MPI Configuration
EXECUTABLE = "./spmv_mpi"      # Your compiled C program
MACHINE_FILE = "machines"      # Your machinefile
OUTPUT_CSV = "ex32_timings.csv"

# 2. Experiment Parameters
# Modify these lists to run the exact cases you need
PROCESS_COUNTS = [60, 64, 120]       # List of Process counts (P)
MATRIX_SIZES = [1000, 10000]         # List of Matrix sizes (N)
SPARSITIES = [0.2, 0.6, 0.9, 0.99]           # List of Sparsities (0.6 = 60% zeros)
ITERATIONS = 20                      # Fixed number of iterations

# ==========================================
# RUNNER FUNCTION
# ==========================================
def run_experiment(procs, n, sparsity, iters):
    """
    Runs the command locally:
    mpiexec -f machines -n <P> ./spmv_mpi <N> <sparsity> <iters>
    """
    cmd = [
        "mpiexec", 
        "-f", MACHINE_FILE, 
        "-n", str(procs), 
        EXECUTABLE, 
        str(n), 
        str(sparsity), 
        str(iters)
    ]

    print(f"Running: P={procs}, N={n}, Sp={sparsity} ...", end=" ", flush=True)

    try:
        # Run command and capture stdout
        result = subprocess.run(cmd, text=True, capture_output=True, check=True)
        output = result.stdout

        # Parse the output using Regex based on your example logs
        timings = {}
        
        # Regex map matching your specific C output format
        patterns = {
            'csr_construct': r"\(i\)\s+CSR Construction Time\s+:\s+([\d\.]+)",
            'csr_comm':      r"\(ii\)\s+CSR Communication Time\s+:\s+([\d\.]+)",
            'csr_calc':      r"\(iii\)\s+CSR Calculation Time\s+:\s+([\d\.]+)",
            'csr_total':     r"\(iv\)\s+Total CSR Time\s+:\s+([\d\.]+)",
            'dense_total':   r"\(v\)\s+Total Dense Time\s+:\s+([\d\.]+)"
        }

        for key, regex in patterns.items():
            match = re.search(regex, output)
            if match:
                timings[key] = float(match.group(1))
            else:
                print(f"[Warning: Could not parse {key}]", end=" ")
                timings[key] = 0.0

        print("DONE.")
        return timings

    except subprocess.CalledProcessError as e:
        print("FAILED.")
        print(f"Error Output:\n{e.stderr}")
        return None
    except Exception as ex:
        print(f"ERROR: {ex}")
        return None

# ==========================================
# MAIN EXECUTION
# ==========================================
if __name__ == "__main__":
    
    # Define CSV Header
    fieldnames = ['N', 'Sparsity', 'Procs', 'Iterations', 
                  'csr_construct', 'csr_comm', 'csr_calc', 'csr_total', 'dense_total']
    
    results = []

    print("--- Starting Local Benchmarks ---")
    print(f"Output file: {OUTPUT_CSV}\n")

    # Loop over all parameter combinations
    for n in MATRIX_SIZES:
        for sp in SPARSITIES:
            for p in PROCESS_COUNTS:
                
                # Execute run
                data = run_experiment(p, n, sp, ITERATIONS)
                
                if data:
                    row = {
                        'N': n,
                        'Sparsity': sp,
                        'Procs': p,
                        'Iterations': ITERATIONS
                    }
                    row.update(data)
                    results.append(row)

    # Save to CSV
    if results:
        with open(OUTPUT_CSV, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=fieldnames)
            writer.writeheader()
            writer.writerows(results)
        print(f"\n[+] Saved {len(results)} entries to {OUTPUT_CSV}")
    else:
        print("\n[-] No results found.")