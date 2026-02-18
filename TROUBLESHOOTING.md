# Troubleshooting

- eBPF load failures: ensure kernel supports BTF and bpftool is installed.
- Missing libraries: check scripts/install_deps.sh and install required -dev packages.
- Permissions: the daemon needs capabilities for eBPF and network monitoring.
- Logs: /var/log/nulleye/nulleye.log and system journal are the first place to look.
