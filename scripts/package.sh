#!/usr/bin/env bash
set -euo pipefail
pkg=nulleye
mkdir -p pkg/DEBIAN pkg/usr/bin pkg/etc pkg/var/lib/nulleye pkg/var/log/nulleye
cp nulleye nulleyed pkg/usr/bin/
cp nulleye.service pkg/etc/
cp config.yaml.example pkg/etc/nulleye/config.yaml
cat > pkg/DEBIAN/control <<'EOF'
Package: nulleye
Version: 0.1.0
Section: utils
Priority: optional
Architecture: amd64
Maintainer: NullEye <nulleye@example.org>
Description: NullEye - host intrusion detection (demo package)
EOF
fakeroot dpkg-deb --build pkg
mv pkg.deb nulleye_0.1.0_amd64.deb

echo "Package created: nulleye_0.1.0_amd64.deb"