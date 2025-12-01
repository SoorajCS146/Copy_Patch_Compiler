# Benchmarking the Copy Patch Compiler

This directory contains scripts and tools to benchmark the Copy Patch Compiler (JIT) against interpreter and native compiler baselines (gcc/clang).

Overview
- `run_bench.sh` — runs the harness: JIT, interpreter, native gcc/clang builds, and hyperfine microbenchmarks. Produces CSVs under `bench/results/`.
- `generate_workloads.sh` — creates toy workloads and native C equivalents under `test/bench/`.
- `parse_results.py` — parses run outputs and hyperfine CSVs and writes `bench/results/summary.csv`.
- `plot_with_pandas.py` — reads `bench/results/summary.csv` and writes PNG charts under `bench/results/`.

Reproducible run (recommended)
1. Use a quiet machine (no heavy background load). Prefer `performance` CPU governor for more stable timings.
2. Install Python deps for plotting (optional):

```bash
python3 -m pip install --user pandas matplotlib
```

3. Ensure `hyperfine` (microbenchmark tool) is installed for more reliable run-time measurements.

4. Generate workloads (if not already generated):

```bash
bash bench/generate_workloads.sh
```

5. Run the benchmark harness (example with extended iterations for hyperfine):

```bash
# run harness (this script will create CSV outputs under bench/results/)
# Some versions of the harness accept an optional HYPERFINE_ITERS environment variable
# to set number of hyperfine runs. If not supported, edit bench/run_bench.sh accordingly.
export HYPERFINE_ITERS=50
bash bench/run_bench.sh
```

6. Parse and plot results:

```bash
python3 bench/parse_results.py
python3 bench/plot_with_pandas.py
```

Output
- `bench/results/summary.csv` — aggregated numbers per workload (compile time, run time, memory, etc.).
- `bench/results/*.png` — charts comparing compile times, run times, memory, and speedups.

Notes & guidance
- If you want confidence intervals/error bars in plots, run with `HYPERFINE_ITERS` >= 30 and then re-run `parse_results.py` and `plot_with_pandas.py`. The plot script will pick up hyperfine CSVs if present.
- For reproducible presentations, re-run the harness several times and take medians; microbenchmark means can still vary across machines.

Contact
- For questions about how the harness measures phases or how the JIT patches stencils at load-time, see `src/main.cpp` and `stencils/print_int.s` in the repository.
