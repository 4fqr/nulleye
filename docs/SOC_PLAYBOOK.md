# SOC Playbook — NullEye

This document provides recommended operational responses and tuning steps for security teams using NullEye.

1) Initial deployment
- Install NullEye on a representative host and verify `/metrics` and `/health`.
- Configure `file_integrity.paths` to include critical system binaries and config directories.

2) Tuning AI thresholds
- Start with `ai.threshold` at 85. Monitor false positives for one week.
- Reduce sensitivity in noisy environments or raise threshold for critical systems.

3) Responding to alerts
- Use the TUI to inspect event context and associated file hashes.
- For confirmed compromises: isolate the host, collect forensic artifacts, and consult the `file_entries` table for recent changes.

4) Rule management
- Define rules conservatively; enable automated responses only after thorough testing.
- Use `rules` to escalate high-confidence anomalies to automated containment actions.

5) Upgrades and backups
- Back up `/var/lib/nulleye/nulleye.db` before upgrades.
- Roll back to the previous DB copy if model or alerting regressions occur.

6) Integration
- Export `/metrics` to Prometheus for long-term alerting.
- Forward alerts to SIEM via a small bridge plugin or external script.

7) Incident checklist
- Preserve logs and DB snapshot
- Quarantine network interfaces
- Capture process memory and disk images
- Rotate credentials and perform post‑incident review

For more detailed operational guidance, see `MANUAL.md` and `README.md`.