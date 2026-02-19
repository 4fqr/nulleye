# NullEye — Host intrusion detection & file-integrity system

NullEye is a modular, production-oriented host intrusion detection system (HIDS) focused on high-fidelity telemetry, low operational overhead, and straightforward operator workflows. It combines optional kernel telemetry (eBPF), continuous file-integrity scanning, a compact anomaly-scoring engine, and an operator-friendly dashboard.

## Key features

- eBPF telemetry (optional): CO-RE BPF programs emit compact events for process, network and filesystem activity
- File-integrity monitoring: SHA-256 + Merkle-root verification and a persistent file inventory
- Anomaly detection: isolation-forest scoring with a sequence LSTM predictor for temporal patterns
- Pluginable: C plugin API plus a Python runner for rapid extension
- Resilient defaults: userspace-only operation, graceful fallbacks for missing system libs
- Small-footprint persistence: SQLite (WAL) for events, alerts and AI model blobs
- Operator UI: ncurses TUI with stdout fallback for constrained environments

---

## Quickstart (developer)

1. Install developer dependencies (Ubuntu):
   sudo ./scripts/install_deps.sh

2. Build userspace-only (no libbpf required):
   make nulleye

3. Run locally (no install):
   ./nulleye            # foreground TUI or stdout fallback
   ./nulleye --daemon   # background service

4. Run unit tests:
   make test

---

## Production install (recommended)

1. Build & install:
   make
   sudo make install

2. Start the service:
   sudo systemctl enable --now nulleye.service

3. Verify:
   sudo journalctl -u nulleye -f
   curl http://localhost:9100/metrics

---

## CLI & runtime diagnostics

- `nulleye --help`      show usage and flags
- `nulleye --version`   print build version and exit
- `nulleye --diag`      print runtime diagnostics and exit
- `nulleye -c <path>`   use a custom config file
- `nulleye -d`          run as daemon

Use `nulleye --diag` to confirm which optional features are compiled and which fallbacks are active.

---

## Configuration (summary)

- File: `/etc/nulleye/config.yaml` (see `config.yaml.example`)
- Important keys and defaults:
  - `database` (string) — SQLite path, default `/var/lib/nulleye/nulleye.db`
  - `log_file` (string) — default `/var/log/nulleye/nulleye.log`
  - `ebpf_ringbuf_size` (int) — event-bus capacity (default: 4096)
  - `file_integrity.paths` (list) — paths to scan (default `/etc`, `/usr/bin`, `/var/www`)
  - `ai.threshold` (int) — anomaly score threshold (default: 85)

If `libyaml` or `libbpf` are not installed, NullEye will still run with sensible defaults.

---

## Operational notes

- Metrics: HTTP `/metrics` (Prometheus text format) on port `9100` by default.
- Logs: file `/var/log/nulleye/nulleye.log` (if writable) and syslog.
- Database: SQLite WAL; back up `/var/lib/nulleye/nulleye.db` before upgrades.
- eBPF: requires kernel BTF + clang/bpftool to build/load programs.

---

## Development & testing

- Build: `make nulleye`
- Tests: `make test` (runs unit tests in `src/tests/`)
- Add eBPF programs: add `.bpf.c` under `src/ebpf/bpf/` (requires clang/bpftool)

---

## Troubleshooting

- eBPF programs not loading: confirm `clang`, `bpftool`, and kernel BTF support.
- TUI not available: ncurses missing — the stdout fallback will still show events.
- DB errors: run with `--diag` to inspect the configured DB path; check file perms and available disk space.

---

## Security

- eBPF program attach operations require elevated privileges; NullEye minimizes privileged work after attach.
- Use the packaged service unit to run with least privilege and proper filesystem locations.

---

## Where to look next

- `ARCHITECTURE.md` — design, dataflow and schema
- `MANUAL.md` — admin guide and configuration examples
- `CONTRIBUTING.md` — dev workflow and coding standards

---

## License

NullEye is available under the MIT License — see `LICENSE`.
