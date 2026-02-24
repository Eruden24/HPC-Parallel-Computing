import subprocess
import csv

# --- PARAMETERS ---
machines_list = [10, 20, 50, 80, 120 ]   # number of MPI processes
degrees = [10**5, 10**6]                   # polynomial degrees
ssh_host = "linux01.di.uoa.gr"             # SSH host
remote_path = "~/parallel_systems"
program = "./polymul"
machines_file = "machines"
output_csv = "mpi_timings.csv"             # CSV output file

# --- FUNCTION TO RUN REMOTE MPI JOB ---
def run_mpi_job(num_machines, degree):
    cmd = f'ssh {ssh_host} "cd {remote_path} && mpiexec -f {machines_file} -n {num_machines} {program} --degree {degree} --time"'
    result = subprocess.run(cmd, shell=True, text=True, capture_output=True)
    
    if result.returncode != 0:
        print(f"Error running {num_machines} processes for degree {degree}:")
        print(result.stderr)
        return None
    
    # Parse timings from stdout
    times = {}
    for line in result.stdout.splitlines():
        if "Data send time" in line:
            times['data_send'] = float(line.split(":")[-1].strip().split()[0])
        elif "Parallel compute time" in line:
            times['compute'] = float(line.split(":")[-1].strip().split()[0])
        elif "Result receive time" in line:
            times['receive'] = float(line.split(":")[-1].strip().split()[0])
        elif "Total execution time" in line:
            times['total'] = float(line.split(":")[-1].strip().split()[0])
    return times

# --- COLLECT DATA AND WRITE CSV ---
with open(output_csv, 'w', newline='') as csvfile:
    fieldnames = ['degree', 'num_processes', 'data_send', 'compute', 'receive', 'total']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    writer.writeheader()
    
    for degree in degrees:
        for num_machines in machines_list:
            print(f"Running degree={degree}, processes={num_machines}...")
            times = run_mpi_job(num_machines, degree)
            if times:
                row = {'degree': degree, 'num_processes': num_machines}
                row.update(times)
                writer.writerow(row)

print(f"All timings saved to {output_csv}")
