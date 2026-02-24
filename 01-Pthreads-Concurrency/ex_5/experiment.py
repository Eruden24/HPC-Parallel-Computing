#!/usr/bin/env python3
import subprocess
import os
import numpy as np
import re
import plotly.graph_objects as go
from plotly.subplots import make_subplots

# ============================================================
#                  EXECUTABLES
# ============================================================

base_dir = os.path.dirname(os.path.abspath(__file__))
executables = {
    "pthread": os.path.join(base_dir, "bin/pthread"),
    "cond":    os.path.join(base_dir, "bin/cond"),
    "sense":   os.path.join(base_dir, "bin/sense")
}

# ============================================================
#                  EXPERIMENT PARAMETERS
# ============================================================

num_threads_list = [2, 4, 8, 16]
num_loops_list   = [a for a in range(10, 201, 10)]

# ============================================================
#                  RESULTS STORAGE
# ============================================================

results = {name: {t: [] for t in num_threads_list} for name in executables}

# ============================================================
#                  RUN EXPERIMENTS
# ============================================================

print("=== Running barrier benchmark experiments ===\n")

for t in num_threads_list:
    for loops in num_loops_list:
        print(f"Running: threads={t}, loops={loops}")
        for name, exe in executables.items():
            try:
                completed = subprocess.run(
                    [exe, str(t), str(loops)],
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    text=True
                )

                match = re.search(r"Total time:\s*([\d\.]+)\s*seconds", completed.stdout)
                if match:
                    elapsed = float(match.group(1))
                else:
                    elapsed = None
                    print(f"[WARN] Could not parse time from {name}")

                results[name][t].append(elapsed)

            except Exception as e:
                print(f"[ERROR] {name} failed:", e)
                results[name][t].append(None)

print("\n=== Experiments completed ===\n")

# ============================================================
#                  COMPUTE AVERAGES
# ============================================================

avg_results = {}
for name in results:
    matrix = []
    for t in num_threads_list:
        row = []
        for idx in range(len(num_loops_list)):
            vals = [results[name][t][idx]]
            vals = [v for v in vals if v is not None]
            row.append(np.mean(vals) if vals else None)
        matrix.append(row)
    avg_results[name] = np.array(matrix)

# ============================================================
#              GLOBAL SCALE FOR ALL AXES
# ============================================================

all_times = []
for name, Z in avg_results.items():
    all_times.extend(Z.flatten())

all_times = [v for v in all_times if v is not None]

global_min = min(all_times)
global_max = max(all_times)

min_threads = min(num_threads_list)
max_threads = max(num_threads_list)
min_loops   = min(num_loops_list)
max_loops   = max(num_loops_list)

print("\nGlobal Z scale:", global_min, "→", global_max)

# ============================================================
#           SIDE-BY-SIDE 3D HEATMAPS — GLOBAL COLORSCALE
#           BUT AUTO AXIS SCALING
# ============================================================

from plotly.subplots import make_subplots

fig = make_subplots(
    rows=1,
    cols=3,
    specs=[[{'type': 'surface'}, {'type': 'surface'}, {'type': 'surface'}]],
    horizontal_spacing=0.05,
    subplot_titles=("PTHREAD Barrier", "COND Barrier", "SENSE Barrier")
)

color_map = "Turbo"  # scientific heatmap

T, L = np.meshgrid(num_threads_list, num_loops_list)

for i, (name, Z) in enumerate(avg_results.items(), start=1):

    fig.add_trace(
        go.Surface(
            z=Z.T,
            x=T,
            y=L,
            colorscale=color_map,
            cmin=global_min,       # keep global heatmap scale
            cmax=global_max,
            showscale=(i == 1),    # only first shows colorbar
            colorbar=dict(title="Time (s)") if i == 1 else None,
            opacity=0.95
        ),
        row=1, col=i
    )

    # remove fixed axis ranges → automatic scaling
    fig.update_scenes(
        dict(
            xaxis_title="Threads",
            yaxis_title="Iterations",
            zaxis_title="Avg Time (s)"
        ),
        row=1, col=i
    )

fig.update_layout(
    title="Barrier Performance Comparison — Side-by-Side 3D Heatmaps",
    width=2100,
    height=750
)

fig.write_html("barrier_comparison.html")
print("\nSaved to barrier_comparison.html")

fig.show()
