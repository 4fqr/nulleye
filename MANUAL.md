# NullEye Manual

Overview

NullEye is an agent designed to provide host telemetry, file integrity monitoring and real-time anomaly detection. This manual covers installation, configuration and operation for administrators.

Installation

1. Install dependencies:
   sudo ./scripts/install_deps.sh
2. Build:
   make
3. Install:
   sudo make install
4. Start:
   sudo systemctl enable --now nulleye.service

Configuration

- The main configuration file is /etc/nulleye/config.yaml. Use config.yaml.example as a template.
- Profiles allow separate monitoring sets; include other YAML files using path references.

Runtime

- The daemon runs as `nulleyed`. The terminal UI `nulleye` connects over a Unix socket.
- Metrics are exposed on port 9100 at `/metrics` by default.

Backups and upgrades

- Back up /var/lib/nulleye/nulleye.db before upgrades.
- To upgrade, stop the service, replace binary, then start the service.

Support

- Check /var/log/nulleye/nulleye.log and `journalctl -u nulleye` for runtime information.
