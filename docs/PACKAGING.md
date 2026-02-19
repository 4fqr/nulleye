# Packaging and distribution — NullEye

This document explains how to build, test, package and distribute NullEye for production.

Supported artifacts

- Native Debian package (.deb) — installs binaries, systemd unit and example config
- Docker image — containerized runtime for environments where kernel eBPF is not required or where host devices/capabilities can be provided
- GitHub release artifacts — binary + .deb + Docker image (GHCR)

Build a .deb locally

1. Build the project:
   git clone https://github.com/4fqr/nulleye.git
   cd nulleye
   make nulleye

2. Produce a Debian package (versioned):
   ./scripts/package.sh 0.1.0
   sudo dpkg -i nulleye_0.1.0_amd64.deb

What the .deb contains

- /usr/bin/nulleye, /usr/bin/nulleyed
- /etc/nulleye/config.yaml (example)
- /etc/systemd/system/nulleye.service
- /usr/lib/nulleye/plugins/ (empty dir for plugins)
- Postinst/prerm scripts to enable/disable the systemd unit

Notes on signing and repositories

- To publish a .deb into an APT repo, sign the package and add it to your repository (e.g., reprepro or aptly).
- For distribution in private environments, upload the .deb to internal artifact storage and use your infrastructure's deployment pipelines.

Build & run Docker image

1. Build:
   make docker-image
   docker images | grep nulleye

2. Run (userspace-only mode — eBPF requires host tooling & capabilities):
   docker run --rm -p 9100:9100 nulleye:local

3. Run with BPF device + capabilities (only on privileged host):
   docker run --rm -p 9100:9100 --cap-add SYS_ADMIN --cap-add NET_ADMIN --device /dev/bpf nulleye:local

Publish Docker image (CI)

- GitHub Actions workflow (`release-on-tag`) builds and pushes images to `ghcr.io/<owner>/nulleye:<tag>` when you create a git tag.
- To publish to Docker Hub, add a publish job with credentials (not included by default).

CI & Release automation

- `CI` workflow builds userspace and runs unit tests on push/PR to `main`.
- Tagged releases trigger `release-on-tag`: builds artifacts, creates release and publishes Docker image to GHCR.

Security & hardening

- The packaged systemd unit is configured to run `nulleye` as a service; review unit options and consider sandboxing using systemd `ProtectSystem`/`PrivateTmp` as required.
- Docker images should be scanned and run with minimal capabilities in production; only add `SYS_ADMIN`/`NET_ADMIN` when host-level telemetry is required.

Troubleshooting

- If dpkg reports missing dependencies, install them via `apt-get install -f` and re-run `dpkg -i`.
- If Docker image needs host access to BPF and fails, verify kernel BTF and host `bpftool` availability.

Advanced topics

- Signed packages: use `dpkg-sig` or `debsign` before publishing to repositories.
- Continuous delivery: integrate `scripts/package.sh` into your pipeline and push generated `.deb` artifacts into your internal repo.

See also: `README.md`, `MANUAL.md`, `.github/workflows/ci.yml`.