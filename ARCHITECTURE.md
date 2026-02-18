# NullEye — Architecture

Purpose

NullEye provides host-level visibility by combining kernel telemetry, persistent storage, and an operator-friendly terminal dashboard. It is designed to be modular, auditable, and suitable for production hardening.

Components

- Core
  - Event bus: lock-free multi-producer / multi-consumer ring buffer for in-memory event exchange between modules and the TUI.
  - Configuration: YAML-driven configuration with sensible defaults and profile support.
  - Storage: SQLite (WAL) for events, file inventory, alerts and AI model blobs.
  - Logger: multi-channel (file and syslog) with an in-memory ring for the TUI.

- eBPF (kernel telemetry)
  - Programs attach to tracepoints (execve, connect, etc.) and emit compact events to a BPF ring buffer.
  - Userspace loader uses libbpf, supports CO-RE when available, and falls back gracefully if unsupported.

- AI Engine
  - Lightweight isolation-forest implementation for online anomaly scoring of process events.
  - Model persists in SQLite and updates on a sliding-window basis to remain adaptive.
  - Explainability: simple feature contributions for operator context.

- Modules and Plugins
  - Built-in modules: FileIntegrity, ProcessMonitor, NetworkMonitor, UserMonitor, AIEngine.
  - Plugin API: modules expose init/fini/process_event/tui_draw callbacks and may be loaded from /usr/lib/nulleye/plugins.

- TUI
  - ncurses-based dashboard with configurable panels (events, process tree, network map, graphs, alerts).
  - Connects to the daemon over a Unix socket for read-only UI operations.

Data flow

1. eBPF programs emit events to a BPF ring buffer.
2. Userspace loader reads events, normalizes them and publishes to the core event bus.
3. Consumers (AI engine, DB writer, TUI) subscribe to the event bus and act accordingly.
4. Alerts and model artifacts are persisted in SQLite.

Operational considerations

- Privilege separation: loader requires elevated privileges only for attach time; other components run unprivileged when possible.
- Resilience: modules run in isolated worker threads; persistent state is stored and WAL-protected.
- Extensibility: new telemetry sources and panels can be added via the plugin API.

Schema summary (SQLite)

- events(id, timestamp, module, data BLOB)
- file_entries(path, hash, mtime, size, perms, uid, gid, attrs)
- alerts(id, timestamp, severity, module, message, acknowledged)
- model(key, value BLOB)

Maintenance

- Back up the SQLite file before major upgrades.
- Monitor the daemon log and health checks exposed by the service.
