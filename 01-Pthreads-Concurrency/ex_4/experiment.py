import subprocess
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

# -----------------------------------------------------
BANK = "./bank_sim"

account_sizes = [10**2, 10**3, 10**4, 10**5]
transactions = [10**2, 10**3, 10**4]
query_ratios = [0, 20, 50, 80, 100]

PARALLEL_THREADS = 4
ACCOUNTS = sum(account_sizes) // len(account_sizes) 
TRANSACTIONS = sum(transactions) // len(transactions)
QUERY_RATIO = 50

# -----------------------------------------------------
# PARAMETER SWEEPS
# -----------------------------------------------------


# -----------------------------------------------------
# MODES
# -----------------------------------------------------
modes = [
    (0, 0, "Coarse + Mutex"),
    (0, 1, "Coarse + RWLock"),
    (1, 0, "Fine + Mutex"),
    (1, 1, "Fine + RWLock"),
]

# -----------------------------------------------------
sleep_flags = [0, 1]  # 0 = no sleep, 1 = sleep inside critical section

# -----------------------------------------------------
OUTPUT_DIR = "heatmaps"
os.makedirs(OUTPUT_DIR, exist_ok=True)

def run(numAcc, numTrans, percentQ, gran, lockType, threads, sleepFlag):
    cmd = [BANK, str(numAcc), str(numTrans), str(percentQ), str(gran), str(lockType), str(threads), str(sleepFlag)]
    out = subprocess.check_output(cmd).decode()
    for line in out.splitlines():
        if "Exec time:" in line:
            return float(line.split()[2])
    return None

# =====================================================================
# RUN PARALLEL EXPERIMENTS
# =====================================================================
for sleepFlag in sleep_flags:
    print(f"\n========== Experiments with sleep_flag={sleepFlag} ==========")
    all_results = []

    for gran, lock, label in modes:
        print(f"\nRunning parallel mode: {label}")

        # Accounts sweep
        for acc in account_sizes:
            t = run(acc, TRANSACTIONS, QUERY_RATIO, gran, lock, PARALLEL_THREADS, sleepFlag)
            all_results.append([label, "accounts", acc, t])

        # Transactions sweep
        for tr in transactions:
            t = run(ACCOUNTS, tr, QUERY_RATIO, gran, lock, PARALLEL_THREADS, sleepFlag)
            all_results.append([label, "transactions", tr, t])

        # Query sweep
        for q in query_ratios:
            t = run(ACCOUNTS, TRANSACTIONS, q, gran, lock, PARALLEL_THREADS, sleepFlag)
            all_results.append([label, "queries", q, t])

    df_parallel = pd.DataFrame(all_results, columns=["mode", "experiment", "parameter", "exec_time"])
    print(df_parallel)

    # =========================
    # HEATMAPS
    # =========================
    sns.set(style="whitegrid")
    for exp in ["accounts", "transactions", "queries"]:
        sub = df_parallel[df_parallel["experiment"] == exp]
        pivot = sub.pivot(index="mode", columns="parameter", values="exec_time")

        plt.figure(figsize=(10, 5))
        sns.heatmap(pivot, annot=True, fmt=".3f", cmap="YlGnBu")
        plt.title(f"Execution Time Heatmap — {exp} (sleep_flag={sleepFlag})")
        plt.xlabel("Parameter value")
        plt.ylabel("Lock mode")
        plt.tight_layout()

        # Save figure
        filename = f"{OUTPUT_DIR}/heatmap_{exp}_sleep{sleepFlag}.png"
        plt.savefig(filename)
        plt.close()
        print(f"Saved heatmap to {filename}")
