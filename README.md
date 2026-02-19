# NullEye — quick tutorial and reference

This README is a hands‑on tutorial first, then a compact reference for operators and developers. It gets you from zero to a working NullEye instance, then shows how to extend, package and harden the deployment.

Why NullEye?
- Low‑overhead host telemetry with optional kernel eBPF for high‑fidelity events
- Continuous file integrity monitoring with a Merkle-root approach
- Lightweight anomaly detection (isolation‑forest + LSTM) and programmable response rules
- Modular plugin architecture for quick extension (C + Python)

Table of contents
- Quick tutorial (5 minutes)
- Verify & debug (health/metrics/tests)
- Configuration & rules (practical examples)
- Plugins & extending
- Packaging & Docker
- Security, hardening & troubleshooting
- Developer notes and contribution

---

## Quick tutorial (5 minutes)

Prerequisites (Ubuntu):
- Userspace-only: `gcc make libsqlite3-dev libssl-dev`
- Full (optional, for eBPF): `clang bpftool libbpf-dev libelf-dev`

1) Build userspace quickly:

```bash
git clone https://github.com/4fqr/nulleye.git
cd nulleye
make nulleye
```

2) Run the agent (stdout TUI fallback):

```bash
./nulleye        # foreground dashboard
# or run as daemon
./nulleye --daemon
```

3) Confirm basic health and metrics:

```bash
./nulleye --diag           # prints runtime diagnostics
curl http://127.0.0.1:9100/health   # {"status":"ok"}
curl http://127.0.0.1:9100/metrics  # Prometheus metrics
```

4) Run tests (local validation):

```bash
make test
```

5) Quick rules example (detect & notify):

```bash
cat > /tmp/rules.conf <<'EOF'
if contains('ssh') then notify
if anomaly_score>90 then kill
EOF
# load manually in code (or put under /etc/nulleye/) and restart
```

Expected result: matching events will appear in the UI and alerts are recorded in the DB.

---

## Verify & debug

- `nulleye --diag` — displays version, DB path, compiled feature flags and event-bus stats
- Logs: `/var/log/nulleye/nulleye.log` (if writable) and `journalctl -u nulleye`
- Unit tests: `make test`
- Sanitizer build: `make asan` (for local UB/memory checks)

---

## Configuration & rules (practical)

Primary config: `/etc/nulleye/config.yaml` (example provided).

Important snippet (example):
```yaml
database: /var/lib/nulleye/nulleye.db
log_file: /var/log/nulleye/nulleye.log
file_integrity:
  paths:
    - /etc
    - /usr/bin
    - /var/www
ai:
  threshold: 85
```

Rules DSL (examples)
- `if anomaly_score>80 then kill` — high-confidence automated response (use with care)
- `if contains('passwd') then notify` — textual payload match
- `if comm==sshd then notify` — command name match

Authoring tips
- Test rules with `notify` before enabling `kill`/`block_ip`.
- Keep rules conservative; tune `ai.threshold` using historical data.

---

## Plugins & extending

- C plugin: implement `nulleye_module_get()` returning `nuleye_module_t` callbacks; drop in `/usr/lib/nulleye/plugins/`.
- Python plugin: simple scripts consumed by the Python runner (see `src/plugins/example_python_plugin.py`).
- Example workflow: write plugin -> compile -> copy to plugin dir -> restart daemon -> verify in TUI.

See `docs/PLUGINS.md` for a developer guide and code skeletons.

---

## Packaging & Docker

Build a `.deb` locally:

```bash
make nulleye
./scripts/package.sh 0.1.0
sudo dpkg -i nulleye_0.1.0_amd64.deb
```

Build and run Docker image:

```bash
make docker-image
docker run --rm -p 9100:9100 nulleye:local
# for host-level telemetry you must provide capabilities and /dev/bpf
```

CI publishes `.deb` and Docker images for tagged releases (see `.github/workflows/ci.yml`).

---

## Security & hardening (practical checklist)

- Run `nulleye` with a dedicated, unprivileged account where possible.
- Protect filesystem locations: `/var/lib/nulleye` and `/var/log/nulleye` with proper ACLs.
- For systemd, enable sandboxing options: `ProtectSystem=full`, `PrivateTmp=yes` (verify functionality).
- Use `rules` conservatively for automatic responses; log and review all automated actions.

---

## Troubleshooting — common issues & fixes

1) eBPF programs not loading
   - Confirm `clang`, `bpftool` and `libbpf-dev` are installed and kernel BTF is available.
   - Check `journalctl -k` for verifier errors and `journalctl -u nulleye` for loader logs.

2) DB failures
   - Check `/var/lib/nulleye` permissions and available disk space.
   - NullEye falls back to an in-memory DB on startup failure (`--diag` shows DB path).

3) TUI fallback
   - If ncurses is missing, NullEye uses a plain stdout dashboard; install `libncurses-dev` for the full UI.

---

## Developer notes

- Language: C11 + pthreads; unit tests in `src/tests/`.
- eBPF: add CO‑RE-safe `.bpf.c` files under `src/ebpf/bpf/` and ensure clang/bpftool are available at build time.
- CI: runs unit tests, ASAN build, static analysis and publishes artifacts on tags.

---

## Getting help & contributing

- Read `CONTRIBUTING.md` for development workflow.
- Open GitHub issues for bugs/feature requests; use PRs against `main` or feature branches.
- For SOC/playbook and runbooks see `docs/SOC_PLAYBOOK.md`.

---

## Quick reference (commands)

- Build: `make nulleye`
- Test: `make test`
- Package: `./scripts/package.sh <version>`
- Docker: `make docker-image`
- Diagnostics: `nulleye --diag`, `curl /health`, `curl /metrics`

---

If you want, I will now:
1) Open a PR-sized change with this README and a short changelog, or
2) Commit directly to `main` (you selected `main`).

Which do you prefer?