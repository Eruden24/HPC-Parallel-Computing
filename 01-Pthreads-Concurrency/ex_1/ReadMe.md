## Naive Multiplication

Compile:

```bash
make
```

Run:

### Sequential 

```b
./polymul --mode sequential --degree 100000 --time 
Polynomial generation time: 0.009024 s
Sequential multiplication time: 1.890369 s
```

### Parallel 

```bash
./polymul --mode parallel --threads 4 --degree 100000 --time
Polynomial generation time: 0.009410 s
Preprocessing time: 0.000894 s
Parallel multiplication time: 0.826332 s
```

### Run Benchmark

```bash
> python3 benchmark.py
```