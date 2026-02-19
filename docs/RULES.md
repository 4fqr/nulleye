# Rules DSL & Response Engine — NullEye

Overview

NullEye supports a compact rule language to map detected conditions to automated or operator actions. Rules are read from a plain-text file (default path configurable) and evaluated against event payloads.

Rule file location

- Default rules file location: `/etc/nulleye/rules.conf` (example path in `config.yaml.example`)
- Load with `rules_load_from_file(path)` or add via management UI (future)

Syntax (simple)

- Each rule is a single-line `if <condition> then <action>`
- Example:
  - `if anomaly_score>80 then kill`
  - `if contains('ssh') then notify`

Supported condition operators (current)

- `anomaly_score>NN` — numeric comparison against an `score=` field found in the event payload
- `contains('text')` — payload contains the given literal substring (single or double quotes supported)
- `comm==<name>` — command name contains `<name>` (case-sensitive substring match)

Notes

- Conditions currently evaluate against fields embedded in the `event_blob` payload (string/ASCII form). Future releases will expose structured fields for more robust matching.
- If multiple rules match a single event, all matching actions are executed (order preserved).

Actions (built-in)

- `kill` — request the response engine to terminate the offending process (logged, audited)
- `block_ip` — request response engine to add a temporary block for the implicated IP (requires network monitor context)
- `notify` — record an alert and surface to TUI/DB

Safety & testing

- Start with `notify` actions only. Test with `rules_evaluate_and_act()` in a non-privileged environment before enabling `kill` or network-blocking actions.
- Rules are evaluated synchronously inside consumer threads — long-running actions should be deferred to response handlers.

Authoring tips

- Use conservative thresholds to reduce false positives.
- Tune AI model thresholds (see `ai.threshold`) and verify rule matches with `nulleye --diag`.

Extending

- The rules engine is intentionally simple; for complex logic integrate a plugin that subscribes to events and enforces richer policies.

See also: `MANUAL.md`, `ARCHITECTURE.md`, `docs/SOC_PLAYBOOK.md`.