#!/usr/bin/env python3
import sys
from pathlib import Path

P = Path('bench/results/summary.csv')
if not P.exists():
    print('summary.csv not found; run bench/parse_results.py first')
    sys.exit(1)

try:
    import pandas as pd
    import matplotlib.pyplot as plt
except Exception as e:
    print('Required plotting libraries not found (pandas, matplotlib).')
    print('Activate your virtualenv with them installed and run this script again.')
    raise

df = pd.read_csv(P)

# Fill NaNs with zeros for plotting
df[['compile_ms','run_ms']] = df[['compile_ms','run_ms']].fillna(0.0)

out_dir = Path('bench/results')
out_dir.mkdir(parents=True, exist_ok=True)

# Compile time bar
plt.figure(figsize=(8,4))
plt.bar(df['workload'], df['compile_ms'], color='#4C78A8')
plt.ylabel('Compile time (ms)')
plt.title('JIT compile time per workload')
plt.tight_layout()
plt.savefig(out_dir / 'compile_time.png')
print('Wrote', out_dir / 'compile_time.png')

# Run time bar
plt.figure(figsize=(8,4))
plt.bar(df['workload'], df['run_ms'], color='#F58518')
plt.ylabel('Run time (ms)')
plt.title('JIT run time per workload')
plt.tight_layout()
plt.savefig(out_dir / 'run_time.png')
print('Wrote', out_dir / 'run_time.png')

# Compare user CPU times across backends if columns exist
cols = df.columns.tolist()
available = {}
if 'jit_user_s' in cols:
    available['JIT'] = df['jit_user_s'].fillna(0.0)
if 'interp_user_s' in cols:
    available['Interp'] = df['interp_user_s'].fillna(0.0)
if 'gcc_user_s' in cols:
    available['GCC'] = df['gcc_user_s'].fillna(0.0)
if 'clang_user_s' in cols:
    available['Clang'] = df['clang_user_s'].fillna(0.0)

if available:
    plt.figure(figsize=(10,5))
    x = range(len(df))
    width = 0.18
    for i,(k,v) in enumerate(available.items()):
        plt.bar([xx + i*width for xx in x], v, width=width, label=k)
    plt.xticks([xx + width for xx in x], df['workload'])
    plt.ylabel('User CPU time (s)')
    plt.title('User CPU time comparison')
    plt.legend()
    plt.tight_layout()
    plt.savefig(out_dir / 'user_cpu_comparison.png')
    print('Wrote', out_dir / 'user_cpu_comparison.png')

# Memory comparison (JIT peak RSS)
if 'jit_mem_kb' in cols:
    plt.figure(figsize=(8,4))
    plt.bar(df['workload'], df['jit_mem_kb'].fillna(0).astype(int), color='#54A24B')
    plt.ylabel('Peak RSS (KB)')
    plt.title('JIT peak memory by workload')
    plt.tight_layout()
    plt.savefig(out_dir / 'memory_jit.png')
    print('Wrote', out_dir / 'memory_jit.png')

print('Plotting complete.')

# Speedup of Clang (wall) over JIT run time
if 'clang_wall' in cols:
    # convert clang_wall strings like '0:00.01' or '1.23' into seconds
    def wall_to_seconds(s):
        if pd.isna(s):
            return 0.0
        s = str(s)
        if ':' in s:
            parts = s.split(':')
            parts = [float(p) for p in parts]
            # support mm:ss.ms or hh:mm:ss
            if len(parts) == 2:
                return parts[0]*60.0 + parts[1]
            elif len(parts) == 3:
                return parts[0]*3600.0 + parts[1]*60.0 + parts[2]
        try:
            return float(s)
        except:
            return 0.0

    clang_wall_s = df['clang_wall'].apply(wall_to_seconds) if 'clang_wall' in df.columns else None
    jit_run_s = df['run_ms'].astype(float) / 1000.0
    if clang_wall_s is not None:
        speedup = clang_wall_s / (jit_run_s.replace(0, pd.NA))
        speedup = speedup.fillna(0.0)
        plt.figure(figsize=(8,4))
        plt.bar(df['workload'], speedup, color='#8E44AD')
        plt.ylabel('Speedup (Clang wall / JIT run)')
        plt.title('Clang vs JIT: speedup (larger = clang faster)')
        plt.tight_layout()
        plt.savefig(out_dir / 'speedup_clang_over_jit.png')
        print('Wrote', out_dir / 'speedup_clang_over_jit.png')

        # Print textual table
        print('\nSummary table (Clang wall s, JIT run s, speedup):')
        for w, cw, jr, sp in zip(df['workload'], clang_wall_s, jit_run_s, speedup):
            print(f"{w}: clang_wall={cw:.6f}s, jit_run={jr:.6f}s, speedup={sp:.2f}x")
