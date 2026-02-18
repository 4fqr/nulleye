#!/usr/bin/env bash
set -euo pipefail
sudo apt update
sudo apt install -y build-essential clang llvm libelf-dev libbpf-dev bpftool libncurses-dev libssl-dev libyaml-dev libsqlite3-dev libjansson-dev pkg-config

echo "Dependencies installed (you may need to enable kernel headers / BTF for full eBPF support)."