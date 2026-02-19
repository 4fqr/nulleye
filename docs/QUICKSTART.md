# Quickstart — NullEye (developer & operator)

This quickstart walks you through installing, running, and validating NullEye in a local environment.

Prerequisites

- Ubuntu 22.04/24.04 (or similar Linux)
- Build tools: gcc, make
- Optional (for eBPF): clang, bpftool, libbpf-dev, libelf-dev

Local quick run (userspace-only)

1. Clone the repository and build the userspace binary:
   git clone https://github.com/4fqr/nulleye.git
   cd nulleye
   make nulleye

2. Run the agent locally (stdout TUI fallback):
   ./nulleye

3. Run as a background service (daemon mode):
   ./nulleye --daemon

4. Check metrics / health:
   curl http://127.0.0.1:9100/metrics
   curl http://127.0.0.1:9100/health

Validate (unit tests)

- Run the test suite:
  make test

Upgrading to full eBPF

- Install the eBPF toolchain and kernel BTF support, then build:
  sudo apt install clang bpftool libbpf-dev libelf-dev -y
  make

Support & troubleshooting

- Logs: /var/log/nulleye/nulleye.log
- Service: systemctl status nulleye
- Diagnostics: nulleye --diag
