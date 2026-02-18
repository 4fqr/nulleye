#!/usr/bin/env bash
set -euo pipefail
for f in src/ebpf/bpf/*.bpf.c; do
  obj=${f%.c}.o
  clang -O2 -g -target bpf -c "$f" -o "$obj"
done

echo "BPF objects compiled."