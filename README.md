# NullEye — Host intrusion detection and file integrity monitoring

Overview

NullEye is a modular host intrusion detection and file integrity monitoring system designed for reliability and low overhead on Ubuntu 22.04/24.04. It combines kernel telemetry (eBPF), a compact anomaly-detection engine, persistent storage, and a terminal-based dashboard for operators.

Prerequisites

- Ubuntu 22.04 or 24.04
- Recommended developer packages (for full feature set): clang, llvm, bpftool, libbpf-dev, libelf-dev
- Runtime / userspace-only build: libssl-dev, libsqlite3-dev (libyaml-dev and libncurses-dev are optional)

Notes

- eBPF is optional at runtime: NullEye will run without kernel telemetry if libbpf/bpf toolchain is not available.
- The build system supports userspace-only compilation via `make nulleye` (no libbpf required).


Installation (developer flow)

1. Install system dependencies:
   sudo ./scripts/install_deps.sh
2. Build the project:
   make
3. Install binaries and service files:
   sudo make install
4. Start the daemon:
   sudo systemctl enable --now nulleye.service
5. Run the TUI client (connects to the running daemon):
   nulleye

Configuration

- Default config path: /etc/nulleye/config.yaml
- Example config provided at config.yaml.example
- Key sections: profiles, file_integrity, ai, ebpf, socket, database, log_file

Basic concepts

- Event bus: lock-free ring buffer; all modules publish/consume structured events.
- eBPF telemetry: tracepoints deliver process and network events into the ring buffer.
- AI engine: in-memory isolation forest trained on recent events; stores model in SQLite.
- Plugins: modules implement a small C API and can be loaded at runtime.

Development notes

- Code is standard C11 with GNU extensions. Compiler flags in the Makefile enable strict warnings.
- Tests: see src/tests/
- To add an eBPF program: add a *.bpf.c file under src/ebpf/bpf and add it to the Makefile.

Troubleshooting

- If eBPF programs fail to load, confirm kernel BTF and bpftool availability.
- Check logs under /var/log/nulleye/nulleye.log and the system journal for the service.

Security

- The eBPF loader requires root to attach programs; the main loop drops privileges where appropriate.
- SQLite WAL is enabled for durability; model blobs are stored in the database.

License

NullEye is provided under the MIT License. See LICENSE for details.
