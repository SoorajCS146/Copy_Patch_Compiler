#!/usr/bin/env bash
set -euo pipefail

# Simple benchmark harness for Copy Patch Compiler
# Measures end-to-end using /usr/bin/time -v and runtime with hyperfine when available.

BENCH_DIR="$(pwd)/test/bench"
OUT_DIR="bench/results"
mkdir -p "$OUT_DIR"

WORKLOADS=(
  "$BENCH_DIR/tiny.toy"
  "$BENCH_DIR/small.toy"
  "$BENCH_DIR/medium.toy"
  "$BENCH_DIR/large.toy"
)

# Commands to compare:
# 1) JIT (Copy Patch Compiler)
# 2) Interpreter (same binary with --mode=interp)
# 3) Native C (compiled with gcc -O2)

for f in "${WORKLOADS[@]}"; do
  base=$(basename "$f" .toy)
  echo "=== Benchmarking $base ==="

  # End-to-end / peak memory for JIT
  echo "JIT end-to-end & peak memory:" > "$OUT_DIR/${base}-jit-time.txt"
  # run JIT with BENCH_PHASES=1 to capture GEN_START/END and RUN_START/END
  BENCH_PHASES=1 /usr/bin/time -v -- "$PWD/build/stencilc" --mode=jit "$f" 1>>"$OUT_DIR/${base}-jit-out.txt" 2>>"$OUT_DIR/${base}-jit-time.txt" || true

  # End-to-end / peak memory for interpreter
  echo "INTERP end-to-end & peak memory:" > "$OUT_DIR/${base}-interp-time.txt"
  /usr/bin/time -v -- "$PWD/build/stencilc" --mode=interp "$f" 1>>"$OUT_DIR/${base}-interp-out.txt" 2>>"$OUT_DIR/${base}-interp-time.txt" || true

  # Native: build C program if exists
  cc_src="$BENCH_DIR/native/${base}.c"
  if [ -f "$cc_src" ]; then
    # GCC native (measure compile time)
    echo "GCC compile:" > "$OUT_DIR/${base}-native-gcc-compile-time.txt"
    /usr/bin/time -v -o "$OUT_DIR/${base}-native-gcc-compile-time.txt" -- gcc -O2 -o "$OUT_DIR/${base}-native-gcc" "$cc_src" 2>&1 || true
    # GCC -O0 baseline
    echo "GCC -O0 compile:" > "$OUT_DIR/${base}-native-gcc-O0-compile-time.txt"
    /usr/bin/time -v -o "$OUT_DIR/${base}-native-gcc-O0-compile-time.txt" -- gcc -O0 -o "$OUT_DIR/${base}-native-gcc-O0" "$cc_src" 2>&1 || true
    echo "NATIVE(gcc) end-to-end & peak memory:" > "$OUT_DIR/${base}-native-gcc-time.txt"
    /usr/bin/time -v -- "$OUT_DIR/${base}-native-gcc" 1>>"$OUT_DIR/${base}-native-gcc-out.txt" 2>>"$OUT_DIR/${base}-native-gcc-time.txt" || true

    # Clang native (if installed)
    if command -v clang >/dev/null 2>&1; then
      echo "CLANG compile:" > "$OUT_DIR/${base}-native-clang-compile-time.txt"
      /usr/bin/time -v -o "$OUT_DIR/${base}-native-clang-compile-time.txt" -- clang -O2 -o "$OUT_DIR/${base}-native-clang" "$cc_src" 2>&1 || true
      # CLANG -O0 baseline
      echo "CLANG -O0 compile:" > "$OUT_DIR/${base}-native-clang-O0-compile-time.txt"
      /usr/bin/time -v -o "$OUT_DIR/${base}-native-clang-O0-compile-time.txt" -- clang -O0 -o "$OUT_DIR/${base}-native-clang-O0" "$cc_src" 2>&1 || true
      echo "NATIVE(clang) end-to-end & peak memory:" > "$OUT_DIR/${base}-native-clang-time.txt"
      /usr/bin/time -v -- "$OUT_DIR/${base}-native-clang" 1>>"$OUT_DIR/${base}-native-clang-out.txt" 2>>"$OUT_DIR/${base}-native-clang-time.txt" || true
    fi
    # TCC baseline (Tiny C Compiler) - fast compile, lower-quality runtime
    if command -v tcc >/dev/null 2>&1; then
      echo "TCC compile:" > "$OUT_DIR/${base}-native-tcc-compile-time.txt"
      /usr/bin/time -v -o "$OUT_DIR/${base}-native-tcc-compile-time.txt" -- tcc -O0 -o "$OUT_DIR/${base}-native-tcc" "$cc_src" 2>&1 || true
      echo "NATIVE(tcc) end-to-end & peak memory:" > "$OUT_DIR/${base}-native-tcc-time.txt"
      /usr/bin/time -v -- "$OUT_DIR/${base}-native-tcc" 1>>"$OUT_DIR/${base}-native-tcc-out.txt" 2>>"$OUT_DIR/${base}-native-tcc-time.txt" || true
    fi
  fi

  # Runtime microbench with hyperfine (if installed)
  if command -v hyperfine >/dev/null 2>&1; then
    HF_RUNS=${HYPERFINE_ITERS:-5}
    echo "Running hyperfine for runtime measurements (${HF_RUNS} runs)"
    # For hyperfine, use wrapper so JIT runs include BENCH_PHASES env
    # For hyperfine, use wrapper so JIT runs include BENCH_PHASES env
    # Include gcc and clang native binaries if present
    GCC_BIN="$OUT_DIR/${base}-native-gcc"
    CLANG_BIN="$OUT_DIR/${base}-native-clang"
    HF_CMD=( --runs "$HF_RUNS" --export-csv "$OUT_DIR/${base}-hyperfine.csv" )
    HF_CMD+=( "BENCH_PHASES=1 $PWD/build/stencilc --mode=jit $f" "$PWD/build/stencilc --mode=interp $f" )
    if [ -x "$GCC_BIN" ]; then HF_CMD+=( "$GCC_BIN" ); fi
    if [ -x "$OUT_DIR/${base}-native-gcc-O0" ]; then HF_CMD+=( "$OUT_DIR/${base}-native-gcc-O0" ); fi
    if [ -x "$CLANG_BIN" ]; then HF_CMD+=( "$CLANG_BIN" ); fi
    if [ -x "$OUT_DIR/${base}-native-clang-O0" ]; then HF_CMD+=( "$OUT_DIR/${base}-native-clang-O0" ); fi
    TCC_BIN="$OUT_DIR/${base}-native-tcc"
    if [ -x "$TCC_BIN" ]; then HF_CMD+=( "$TCC_BIN" ); fi
    hyperfine "${HF_CMD[@]}"
  fi

  echo "done $base"
done

echo "Results in $OUT_DIR"
