# NullEye — Administrator Manual

This manual documents operational procedures, configuration reference, and troubleshooting steps for NullEye.

## Quick operational checklist

- Install and build: `make && sudo make install`
- Start service: `sudo systemctl enable --now nulleye.service`
- Verify health: `curl http://127.0.0.1:9100/metrics` and `sudo journalctl -u nulleye -f`

## Configuration reference

File: `/etc/nulleye/config.yaml` (YAML)

Example structure (keys and types):

- `database` (string)
  - Path to the SQLite DB. Default `/var/lib/nulleye/nulleye.db`

- `log_file` (string)
  - Log file path. Default `/var/log/nulleye/nulleye.log`

- `ebpf_ringbuf_size` (int)
  - Event bus capacity. Default `4096`.

- `file_integrity` (map)
  - `paths` (list of strings) — directories to scan
  - `interval` (int) — scan interval in seconds

- `ai` (map)
  - `threshold` (int) — anomaly score threshold
  - `trees` (int) — isolation forest ensemble size

- `modules` (map)
  - Enable/disable built-in modules (ai, file_integrity, process_monitor, network_monitor, user_monitor)

Notes

- Missing keys default to safe, production-friendly values.
- If `libyaml` is not present at build time, NullEye falls back to a minimal parser.

## Runtime modes

- Userspace-only: runs without libbpf/clang; useful for lightweight hosts or containers.
- Full mode: when libbpf and kernel BTF are available, eBPF programs provide higher-fidelity events.

## Health & diagnostics

- Runtime diagnostics: `nulleye --diag` (prints version, DB path, compiled features, event-bus stats)
- Metrics: `http://<host>:9100/metrics`
- Logs: `tail -F /var/log/nulleye/nulleye.log` and `journalctl -u nulleye`.

## Backups & upgrades

- Back up the SQLite DB before upgrades: `cp /var/lib/nulleye/nulleye.db /var/lib/nulleye/nulleye.db.bak`
- Upgrade procedure:
  1. `sudo systemctl stop nulleye`
  2. Replace binary (or package install)
  3. `sudo systemctl start nulleye`
  4. Verify logs and metrics

## Troubleshooting

- eBPF programs will not load if clang/bpftool or libbpf-dev are missing — check `dmesg` and `journalctl`.
- If the TUI is blank, run `nulleye --diag` and confirm event-bus pending > 0.
- If DB writes fail, check disk space and file permissions for `/var/lib/nulleye`.

## Security recommendations

- Run the service with a dedicated unprivileged account where possible.
- Limit access to `/var/lib/nulleye` and log directories using file system ACLs.
- Use system-level auditing for sensitive hosts (auditd) in addition to NullEye.

## Administration API (CLI)

- `nulleye --version` — show build version
- `nulleye --diag` — print runtime diagnostics
- `nulleye --help` — usage

## Contact & contribution

- See `CONTRIBUTING.md` for development workflow and pull-request guidelines.
- For security issues, see `SECURITY.md`.
