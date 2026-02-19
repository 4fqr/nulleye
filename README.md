# NullEye — production README

Production‑grade overview

NullEye is a lightweight, extensible Host Intrusion Detection & File Integrity Monitoring system. It pairs optional kernel telemetry (eBPF) with a compact userspace engine (event bus, rules, AI scoring, TUI) and a small persistent store (SQLite WAL). Designed for both beginners and security teams, NullEye ships safe defaults, robust fallbacks, and clear operational controls.

Why use NullEye
- Kernel telemetry (optional) for high‑fidelity process/network visibility
- Continuous file integrity scanning with SHA‑256 + Merkle roots
- Real‑time anomaly scoring (isolation‑forest + LSTM predictor)
- Programmable rule/response engine and pluginable architecture
- Small footprint: single binary, SQLite state, Prometheus metrics

Table of contents
- Quickstart
- Production install (packages & Docker)
- Core concepts & recommended configuration
- Rules, responses & best practices
- Plugin development
- Packaging & CI
- Security & operational hardening
- Troubleshooting & support

---

## Quickstart (5 minutes)

Prerequisites (developer): gcc, make, libsqlite3-dev, libssl-dev

Build and run (userspace-only):

```bash
git clone https://github.com/4fqr/nulleye.git
cd nulleye
make nulleye
./nulleye               # run foreground (ncurses or stdout fallback)
./nulleye --daemon      # background
```

Verify health & metrics:

```bash
./nulleye --diag
curl http://127.0.0.1:9100/health
curl http://127.0.0.1:9100/metrics
```

Run unit tests:

```bash
make test
```

---

## Production install

Package (.deb)

```bash
make
./scripts/package.sh 0.1.0
sudo dpkg -i nulleye_0.1.0_amd64.deb
```

Docker (containerized run):

```bash
make docker-image
docker run --rm -p 9100:9100 nulleye:local
```

Notes
- eBPF requires clang, bpftool and kernel BTF; NullEye runs without eBPF on hosts missing those components.
- Service unit installs to `/etc/systemd/system/nulleye.service` and data to `/var/lib/nulleye`.

---

## Core concepts & recommended configuration

Event bus
- Lock‑free ring buffer for in-memory event exchange between producers (loader, modules) and consumers (AI, DB, TUI).

Database
- SQLite (WAL) for events, file inventory and AI model blobs.

AI
- Isolation‑forest provides anomaly scores; LSTM predictor supports sequence detection.
- Tune `ai.threshold` incrementally using operator feedback.

Defaults and fallbacks
- Missing config/database/log file → NullEye warns and falls back to defaults or in‑memory DB.
- Missing ncurses → stdout TUI fallback.
- Missing libbpf/clang → eBPF disabled but userspace remains fully functional.

Recommended minimal `config.yaml` entries
```yaml
database: /var/lib/nulleye/nulleye.db
log_file: /var/log/nulleye/nulleye.log
ai:
  threshold: 85
file_integrity:
  paths: [/etc, /usr/bin]
```

---

## Rules & response engine (practical)

Rules file (plain text): `/etc/nulleye/rules.conf`
Format: `if <condition> then <action>`

Examples
- `if anomaly_score>90 then kill`  — high-confidence auto‑response (test extensively)
- `if contains('passwd') then notify` — payload substring match

Built‑in actions
- `notify` — raise an alert (safe)
- `kill` — terminate a process (dangerous; test in staging)
- `block_ip` — add a temporary network block (requires network context)

Best practices
- Start with `notify` actions, monitor for false positives, then progressively enable automated responses.
- Use `nulleye --diag` and the TUI to validate rule matches before enabling aggressive actions.

---

## Plugin development (C & Python)

C plugins
- Implement `nuleye_module_get()` returning `nuleye_module_t` with lifecycle callbacks.
- Place compiled `.so` in `/usr/lib/nulleye/plugins/`.

Python plugins
- Use the Python runner; place scripts under the plugin dir and they will be executed with event JSON on stdin.

See `docs/PLUGINS.md` and `src/plugins/example_plugin.c` for examples.

---

## Packaging, CI & releases

- Local packaging: `./scripts/package.sh <version>` → `.deb`
- Docker: `make docker-image`
- CI: GitHub Actions builds userspace, runs tests, ASAN, static analysis and publishes artifacts on tags (binary, .deb, Docker image).

Release process (recommended)
1. Create a release branch and open PR against `main`.
2. Run CI and address issues.
3. Tag `vX.Y.Z` and push — CI will create release assets and push Docker image.

---

## Security & operational hardening

- Run NullEye under a dedicated system account. Limit filesystem access to `/var/lib/nulleye` and `/var/log/nulleye`.
- For systemd units, apply `ProtectSystem`, `PrivateTmp` and similar sandboxing options as appropriate.
- Review and sign release artifacts before deployment to production.

---

## Troubleshooting quick checklist

- eBPF load failure: verify clang / bpftool / libbpf and kernel BTF.
- Logs: `journalctl -u nulleye` and `/var/log/nulleye/nulleye.log`.
- DB errors: check file permissions and disk space; NullEye can run in-memory if DB path is not writable.

---

## Contribution & support

- Report issues and submit PRs on GitHub.
- Follow `CONTRIBUTING.md` for coding standards and commit conventions.

---

## License

MIT — see `LICENSE`.

---

If you'd like, I will now:
- (A) open a review PR for this README on `feature/readme`, or
- (B) commit directly to `main` and push (you previously requested direct commits).
