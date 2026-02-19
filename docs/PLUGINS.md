# Plugins — NullEye plugin development guide

NullEye supports two plugin models:
- Native C plugins (shared objects loaded with `dlopen`)
- Python plugins executed via the Python runner

Plugin discovery

- Default plugin directory: `/usr/lib/nulleye/plugins`
- Plugins must be regular `.so` files (C) or scripts managed by the Python runner

C plugin API (summary)

- A plugin must expose the symbol `nulleye_module_get()` which returns a pointer to a `nuleye_module_t` structure.
- `nuleye_module_t` (summary):
  - `name` (const char *)
  - `init(void)` — optional initialization, return 0 on success
  - `fini(void)` — optional cleanup
  - `process_event(const nuleye_event_t *)` — optional event callback
  - `tui_draw(void *)` — optional UI render hook
  - `start/stop` — optional worker thread lifecycle

Example skeleton (C)

- See `src/plugins/example_plugin.c` for a minimal implementation.
- Compile as `gcc -fPIC -shared -o my_plugin.so my_plugin.c`
- Install to `/usr/lib/nulleye/plugins/` and restart the daemon.

Python plugins

- Place `.py` scripts into `/usr/lib/nulleye/plugins` and use the Python runner (`python_runner.c`) to execute them with an event JSON payload.
- The Python runner runs scripts in a subprocess and returns exit status; plugins should communicate via stdout/stderr and exit codes.

Security considerations

- Plugins run inside the main process space (C plugins) — treat them as trusted code; use the Python runner for less-trusted logic.
- Validate and sandbox plugin inputs and limit file system/network access where appropriate.

Testing & development

- Build and load plugins locally using the `Makefile` or compile separately and `dlopen` from the running daemon.
- Use the TUI or `--diag` to confirm plugin module registration.

See also: `src/plugins/example_plugin.c`, `src/plugins/example_python_plugin.py`, `MANUAL.md`, `ARCHITECTURE.md`.