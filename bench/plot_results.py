#!/usr/bin/env python3
import csv
import matplotlib.pyplot as plt
from pathlib import Path

P = Path('bench/results/summary.csv')
if not P.exists():
    print('summary.csv not found; run bench/parse_results.py first')
    raise SystemExit(1)

rows = []
with P.open() as fh:
    r = csv.DictReader(fh)
    for row in r:
        rows.append(row)

workloads = [r['workload'] for r in rows]
compile_ms = [float(r['compile_ms']) if r['compile_ms'] not in (None,'None','') else 0.0 for r in rows]
run_ms = [float(r['run_ms']) if r['run_ms'] not in (None,'None','') else 0.0 for r in rows]

# write compile time bar
#!/usr/bin/env python3
import csv
import matplotlib.pyplot as plt
from pathlib import Path

P = Path('bench/results/summary.csv')
if not P.exists():
    print('summary.csv not found; run bench/parse_results.py first')
    raise SystemExit(1)

rows = []
with P.open() as fh:
    r = csv.DictReader(fh)
    for row in r:
        rows.append(row)

workloads = [r['workload'] for r in rows]
compile_ms = [float(r['compile_ms']) if r['compile_ms'] not in (None,'None','') else 0.0 for r in rows]
run_ms = [float(r['run_ms']) if r['run_ms'] not in (None,'None','') else 0.0 for r in rows]

# write compile time bar
#!/usr/bin/env python3
import csv
from pathlib import Path

def save_bar_svg(filename, labels, values, title, ylabel):
    # simple SVG bar chart
    w = 800
    h = 400
    margin = 60
    n = len(values)
    if n == 0:
        return
    maxv = max(values) if max(values) > 0 else 1
    bar_w = (w - 2*margin) / (n*1.5)
    gap = bar_w * 0.5
    x0 = margin
    parts = []
    parts.append(f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}">')
    parts.append(f'<rect width="100%" height="100%" fill="white"/>')
    parts.append(f'<text x="{w/2}" y="20" text-anchor="middle" font-size="16">{title}</text>')
    # bars
    for i, val in enumerate(values):
        bx = x0 + i*(bar_w+gap)
        bh = (val / maxv) * (h - 2*margin)
        by = h - margin - bh
        parts.append(f'<rect x="{bx}" y="{by}" width="{bar_w}" height="{bh}" fill="#4C78A8"/>')
        parts.append(f'<text x="{bx + bar_w/2}" y="{h - margin + 15}" font-size="10" text-anchor="middle">{labels[i]}</text>')
        parts.append(f'<text x="{bx + bar_w/2}" y="{by - 5}" font-size="10" text-anchor="middle">{round(val,3)}</text>')
    parts.append(f'<text x="{margin/2}" y="{margin/2}" font-size="10">{ylabel}</text>')
    parts.append('</svg>')
    Path(filename).write_text('\n'.join(parts))

P = Path('bench/results/summary.csv')
if not P.exists():
    print('summary.csv not found; run bench/parse_results.py first')
    raise SystemExit(1)

rows = []
with P.open() as fh:
    r = csv.DictReader(fh)
    for row in r:
        rows.append(row)

workloads = [r['workload'] for r in rows]
compile_ms = [float(r['compile_ms']) if r['compile_ms'] not in (None,'None','') else 0.0 for r in rows]
run_ms = [float(r['run_ms']) if r['run_ms'] not in (None,'None','') else 0.0 for r in rows]

save_bar_svg('bench/results/compile_time.svg', workloads, compile_ms, 'JIT compile time by workload', 'ms')
print('Wrote bench/results/compile_time.svg')
save_bar_svg('bench/results/run_time.svg', workloads, run_ms, 'JIT run time by workload', 'ms')
print('Wrote bench/results/run_time.svg')

# user CPU times
def fval(d,k):
    v = d.get(k)
    return float(v) if v not in (None,'','None') and v != '' else 0.0

jit_user = [fval(r,'jit_user_s') for r in rows]
interp_user = [fval(r,'interp_user_s') for r in rows]
gcc_user = [fval(r,'gcc_user_s') for r in rows]
clang_user = [fval(r,'clang_user_s') for r in rows]

# For simplicity generate a chart of max user time per workload
combined = [max(a,b,c,d) for a,b,c,d in zip(jit_user,interp_user,gcc_user,clang_user)]
save_bar_svg('bench/results/user_jit_vs_native.svg', workloads, combined, 'Max user CPU time per workload (s)', 's')
print('Wrote bench/results/user_jit_vs_native.svg')

# memory
jit_mem = [int(r['jit_mem_kb']) if r.get('jit_mem_kb') not in (None,'','None') and r.get('jit_mem_kb') != '' else 0 for r in rows]
save_bar_svg('bench/results/memory_comparison.svg', workloads, jit_mem, 'JIT peak RSS by workload (KB)', 'KB')
print('Wrote bench/results/memory_comparison.svg')
