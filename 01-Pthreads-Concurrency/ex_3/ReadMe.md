## Compile and Run 

```bash
> make 
gcc -Wall -Wextra -O2 -std=c11 -pthread -o main main.c
> ./main 1000
Allocation & initialization time: 0.000217 seconds
Sequential execution time: 0.000021 seconds
Parallel execution time: 0.000487 seconds
Parallel results match sequential: YES
```

## Run Benchmark

```bash
> python3 benchmark.py
```