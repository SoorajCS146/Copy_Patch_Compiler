#!/usr/bin/env python3
import re
import os
import csv
from pathlib import Path

R = Path('bench/results')

workloads = []
for p in R.iterdir():
    # find unique base names like tiny-jit-out.txt -> tiny
    pass

# collect bases by scanning files
bases = set()
for f in R.iterdir():
    m = re.match(r'^(.*?)-(jit|interp|native)-out', f.name)
    if m:
        bases.add(m.group(1))

bases = sorted(bases)

summary = []
for base in bases:
    jit_out = R / f'{base}-jit-out.txt'
    interp_out = R / f'{base}-interp-out.txt'
    native_gcc_out = R / f'{base}-native-gcc-out.txt'
    native_clang_out = R / f'{base}-native-clang-out.txt'
    jit_time = R / f'{base}-jit-time.txt'
    interp_time = R / f'{base}-interp-time.txt'
    native_gcc_time = R / f'{base}-native-gcc-time.txt'
    native_clang_time = R / f'{base}-native-clang-time.txt'

    def parse_times(outfile):
        if not outfile.exists():
            return None
        s = outfile.read_text()
        # find GEN_START, GEN_END, RUN_START, RUN_END
        nums = {}
        for key in ('GEN_START','GEN_END','RUN_START','RUN_END'):
            m = re.search(rf'{key}:(\d+)', s)
            if m:
                nums[key] = int(m.group(1))
        return nums

    jit_phase = parse_times(jit_out)
    interp_phase = parse_times(interp_out)

    def parse_time_file(tfile):
        if not tfile.exists():
            return None
        txt = tfile.read_text()
        m = re.search(r'User time \(seconds\):\s*([0-9.]+)', txt)
        ut = float(m.group(1)) if m else None
        m = re.search(r'System time \(seconds\):\s*([0-9.]+)', txt)
        st = float(m.group(1)) if m else None
        m = re.search(r'Elapsed \(wall clock\) time.*:\s*([0-9:]+\.?[0-9]*)', txt)
        wall = m.group(1) if m else None
        m = re.search(r'Maximum resident set size \(kbytes\):\s*(\d+)', txt)
        mem = int(m.group(1)) if m else None
        return {'user':ut,'sys':st,'wall':wall,'mem_kb':mem}

    jit_time_stats = parse_time_file(jit_time)
    interp_time_stats = parse_time_file(interp_time)
    native_gcc_time_stats = parse_time_file(native_gcc_time)
    native_clang_time_stats = parse_time_file(native_clang_time)

    # parse compile-time outputs for gcc/clang (written with /usr/bin/time -v -o)
    def parse_compile_time_file(cfile):
        if not cfile.exists():
            return None
        txt = cfile.read_text()
        m = re.search(r'Elapsed \(wall clock\) time.*:\s*([0-9:]+\.?[0-9]*)', txt)
        if not m:
            # sometimes time -v -o writes 'Elapsed (wall clock) time (h:mm:ss or m:ss):'
            m = re.search(r'Elapsed \(wall clock\) time.*:\s*([0-9:]+\.?[0-9]*)', txt)
        wall = m.group(1) if m else None
        return {'wall': wall}

    gcc_compile = parse_compile_time_file(R / f'{base}-native-gcc-compile-time.txt')
    clang_compile = parse_compile_time_file(R / f'{base}-native-clang-compile-time.txt')

    # compute durations from phase markers
    compile_time_ms = None
    run_time_ms = None
    if jit_phase and 'GEN_START' in jit_phase and 'GEN_END' in jit_phase:
        compile_time_ms = (jit_phase['GEN_END'] - jit_phase['GEN_START'])/1000.0
    if jit_phase and 'RUN_START' in jit_phase and 'RUN_END' in jit_phase:
        run_time_ms = (jit_phase['RUN_END'] - jit_phase['RUN_START'])/1000.0

    summary.append({'workload':base,'compile_ms':compile_time_ms,'run_ms':run_time_ms,
                    'jit_time':jit_time_stats,'interp_time':interp_time_stats,'native_gcc_time':native_gcc_time_stats,'native_clang_time':native_clang_time_stats,
                    'gcc_compile': gcc_compile,'clang_compile': clang_compile})

# print CSV
out = R / 'summary.csv'
with out.open('w', newline='') as fh:
    w = csv.writer(fh)
    w.writerow(['workload','compile_ms','run_ms','jit_user_s','jit_sys_s','jit_mem_kb','interp_user_s','interp_sys_s','interp_mem_kb','gcc_user_s','gcc_sys_s','gcc_mem_kb','clang_user_s','clang_sys_s','clang_mem_kb','gcc_compile_wall','clang_compile_wall','gcc_wall','clang_wall'])
    for s in summary:
        row = [s['workload'], s['compile_ms'], s['run_ms']]
        jit = s['jit_time'] or {}
        interp = s['interp_time'] or {}
        gcc = s.get('native_gcc_time') or {}
        clang = s.get('native_clang_time') or {}
        gcc_compile = s.get('gcc_compile') or {}
        clang_compile = s.get('clang_compile') or {}
        # prefer hyperfine mean wall time if available
        hf = R / f"{s['workload']}-hyperfine.csv"
        gcc_wall = gcc.get('wall')
        clang_wall = clang.get('wall')
        if hf.exists():
            try:
                import csv
                with hf.open() as fh:
                    # hyperfine CSV has header 'command,mean,...'
                    rdr = csv.DictReader(fh)
                    cmd_mean = {}
                    for hf_row in rdr:
                        cmd = hf_row.get('command') or hf_row.get('Command') or hf_row.get('command ')
                        mean = hf_row.get('mean') or hf_row.get('Mean')
                        if cmd and mean:
                            # store as float seconds
                            try:
                                cmd_mean[cmd] = float(mean)
                            except Exception:
                                pass
                    # choose preferred commands if present (prefer -O0 variants if available for comparison)
                    # mapping keys we'll look for in order
                    gcc_keys = [f'{base}-native-gcc-O0', f'{base}-native-gcc']
                    clang_keys = [f'{base}-native-clang-O0', f'{base}-native-clang']
                    tcc_key = f'{base}-native-tcc'
                    for k in gcc_keys:
                        for cmd in cmd_mean:
                            if k in cmd:
                                gcc_wall = str(cmd_mean[cmd])
                                break
                        if gcc_wall:
                            break
                    for k in clang_keys:
                        for cmd in cmd_mean:
                            if k in cmd:
                                clang_wall = str(cmd_mean[cmd])
                                break
                        if clang_wall:
                            break
                    if not gcc_wall and tcc_key:
                        for cmd in cmd_mean:
                            if tcc_key in cmd:
                                gcc_wall = str(cmd_mean[cmd])
                                break
            except Exception:
                pass

        row += [jit.get('user'),jit.get('sys'),jit.get('mem_kb'), interp.get('user'),interp.get('sys'),interp.get('mem_kb'), gcc.get('user'),gcc.get('sys'),gcc.get('mem_kb'), clang.get('user'),clang.get('sys'),clang.get('mem_kb'), gcc_compile.get('wall'), clang_compile.get('wall'), gcc_wall, clang_wall]
        w.writerow(row)

print('Wrote', out)
