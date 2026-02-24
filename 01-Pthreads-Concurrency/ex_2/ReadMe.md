### Compile and Run 

```bash
> make 
gcc -Wall -Wextra -O2 -pthread -o main main.c
>./main <threads> <iterations> <choice 0, 1, 2 locking mechanism>
```

## Run Benchmark

```bash
> python3 benchmark.py
```