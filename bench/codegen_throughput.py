#!/usr/bin/env python3
"""Measure codegen/compile throughput by running many small compilations

This script runs a small workload repeatedly for several backends and
measures how many runs per second each backend can perform. It writes
`bench/results/codegen_throughput.csv` and `bench/results/codegen_throughput.png`.
"""
import time
import shutil
import subprocess
from pathlib import Path
import csv

OUT = Path('bench/results')
OUT.mkdir(parents=True, exist_ok=True)

# Workload sources
TOY = Path('test/bench/tiny.toy')
CC_SRC = Path('test/bench/native/tiny.c')

BACKENDS = []
# stencilc JIT and interp
if Path('build/stencilc').exists():
    BACKENDS.append(('stencilc-jit', [str(Path('build/stencilc')), '--mode=jit', str(TOY)]))
    BACKENDS.append(('stencilc-interp', [str(Path('build/stencilc')), '--mode=interp', str(TOY)]))

# native compilers
if shutil.which('gcc'):
    BACKENDS.append(('gcc-O0', ['gcc', '-O0', '-o', '/tmp/ct_out', str(CC_SRC)]))
    BACKENDS.append(('gcc-O2', ['gcc', '-O2', '-o', '/tmp/ct_out', str(CC_SRC)]))
if shutil.which('clang'):
    BACKENDS.append(('clang-O0', ['clang', '-O0', '-o', '/tmp/ct_out', str(CC_SRC)]))
    BACKENDS.append(('clang-O2', ['clang', '-O2', '-o', '/tmp/ct_out', str(CC_SRC)]))
if shutil.which('tcc'):
    BACKENDS.append(('tcc', ['tcc', '-o', '/tmp/ct_out', str(CC_SRC)]))

def run_backend(cmd, iterations):
    # run the given command `iterations` times, measuring total wall time
    start = time.monotonic()
    for i in range(iterations):
        # For compilers that write to /tmp/ct_out, overwrite each time.
        subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    end = time.monotonic()
    return end - start

def main():
    iterations = 200
    results = []
    print(f"Found backends: {[b[0] for b in BACKENDS]}")
    for name, cmd in BACKENDS:
        print(f"Running {name} for {iterations} iterations...")
        try:
            elapsed = run_backend(cmd, iterations)
            rate = iterations / elapsed if elapsed > 0 else 0.0
            results.append((name, iterations, elapsed, rate))
            print(f"{name}: {elapsed:.3f}s total, {rate:.1f} ops/s")
        except subprocess.CalledProcessError:
            print(f"{name} failed; skipping")

    # write CSV
    csvp = OUT / 'codegen_throughput.csv'
    with csvp.open('w', newline='') as fh:
        w = csv.writer(fh)
        w.writerow(['backend','iterations','seconds','ops_per_sec'])
        for r in results:
            w.writerow(r)
    print('Wrote', csvp)

    # plot if matplotlib available
    try:
        import pandas as pd
        import matplotlib.pyplot as plt
    except Exception:
        print('pandas/matplotlib not available; skipping plot')
        return

    df = pd.read_csv(csvp)
    plt.figure(figsize=(8,4))
    plt.bar(df['backend'], df['ops_per_sec'], color='#2ca02c')
    plt.ylabel('Operations (compile/run) per second')
    plt.title('Codegen/Compile Throughput')
    plt.xticks(rotation=45)
    plt.tight_layout()
    outp = OUT / 'codegen_throughput.png'
    plt.savefig(outp)
    print('Wrote', outp)

if __name__ == '__main__':
    main()
