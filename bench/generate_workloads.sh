#!/usr/bin/env bash
set -euo pipefail

OUTDIR="test/bench"
mkdir -p "$OUTDIR/native"

# tiny already exists

echo "Generating small.toy (200 ops)"
cat > "$OUTDIR/small.toy" <<'EOF'
let a = 0;
# perform 200 adds
EOF
for i in $(seq 1 200); do
  echo "let a = a + 1;" >> "$OUTDIR/small.toy"
done

echo "Generating medium.toy (2000 ops)"
cat > "$OUTDIR/medium.toy" <<'EOF'
let x = 0;
# perform 2000 adds
EOF
for i in $(seq 1 2000); do
  echo "let x = x + 1;" >> "$OUTDIR/medium.toy"
done

echo "Generating large.toy (20000 ops)"
cat > "$OUTDIR/large.toy" <<'EOF'
let y = 0;
# perform 20000 adds
EOF
for i in $(seq 1 20000); do
  echo "let y = y + 1;" >> "$OUTDIR/large.toy"
done

# generate native C equivalents that do the same work
cat > "$OUTDIR/native/small.c" <<'EOF'
#include <stdio.h>
int main(void){
    long a = 0;
EOF
for i in $(seq 1 200); do
  echo "    a = a + 1;" >> "$OUTDIR/native/small.c"
done
cat >> "$OUTDIR/native/small.c" <<'EOF'
    printf("%ld\n", a);
    return 0;
}
EOF

cat > "$OUTDIR/native/medium.c" <<'EOF'
#include <stdio.h>
int main(void){
    long x = 0;
EOF
for i in $(seq 1 2000); do
  echo "    x = x + 1;" >> "$OUTDIR/native/medium.c"
done
cat >> "$OUTDIR/native/medium.c" <<'EOF'
    printf("%ld\n", x);
    return 0;
}
EOF

cat > "$OUTDIR/native/large.c" <<'EOF'
#include <stdio.h>
int main(void){
    long y = 0;
EOF
for i in $(seq 1 20000); do
  echo "    y = y + 1;" >> "$OUTDIR/native/large.c"
done
cat >> "$OUTDIR/native/large.c" <<'EOF'
    printf("%ld\n", y);
    return 0;
}
EOF

chmod +x "$OUTDIR/small.toy" "$OUTDIR/medium.toy" "$OUTDIR/large.toy"

echo "Workloads generated: small, medium, large (and native C equivalents)"
