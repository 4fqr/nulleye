#!/usr/bin/env bash
set -euo pipefail

VERSION="${1:-$(git describe --tags --always 2>/dev/null || echo "0.1.0") }"
ARCH="$(dpkg --print-architecture 2>/dev/null || echo amd64)"
PKGDIR="pkg"

rm -rf "$PKGDIR"
mkdir -p "$PKGDIR/DEBIAN" "$PKGDIR/usr/bin" "$PKGDIR/etc/nulleye" "$PKGDIR/var/lib/nulleye" "$PKGDIR/var/log/nulleye" "$PKGDIR/usr/lib/nulleye/plugins"

cp -f nulleye "$PKGDIR/usr/bin/"
if [ -x nulleyed ]; then
  cp -f nulleyed "$PKGDIR/usr/bin/"
else
  # create a lightweight nulleyed copy (daemon flag preserved at runtime)
  cp -f nulleye "$PKGDIR/usr/bin/nulleyed"
fi
install -m 0644 nulleye.service "$PKGDIR/etc/"
install -m 0644 config.yaml.example "$PKGDIR/etc/nulleye/config.yaml"

# post-install: ensure dirs, enable service
cat > "$PKGDIR/DEBIAN/postinst" <<'EOF'
#!/bin/sh
set -e
mkdir -p /var/lib/nulleye /var/log/nulleye /usr/lib/nulleye/plugins
chown root:root /var/lib/nulleye /var/log/nulleye
systemctl daemon-reload || true
if [ -x /bin/systemctl ]; then
  systemctl enable nulleye.service >/dev/null 2>&1 || true
  systemctl restart nulleye.service >/dev/null 2>&1 || true
fi
exit 0
EOF
chmod 0755 "$PKGDIR/DEBIAN/postinst"

# pre-remove: stop service
cat > "$PKGDIR/DEBIAN/prerm" <<'EOF'
#!/bin/sh
set -e
if [ -x /bin/systemctl ]; then
  systemctl stop nulleye.service >/dev/null 2>&1 || true
  systemctl disable nulleye.service >/dev/null 2>&1 || true
fi
exit 0
EOF
chmod 0755 "$PKGDIR/DEBIAN/prerm"

# control file
cat > "$PKGDIR/DEBIAN/control" <<EOF
Package: nulleye
Version: ${VERSION}
Section: utils
Priority: optional
Architecture: ${ARCH}
Maintainer: NullEye <nulleye@example.org>
Depends: libsqlite3-0, libc6, libssl1.1
Description: NullEye - host intrusion detection and file integrity monitoring
 NullEye provides modular host monitoring with optional kernel telemetry (eBPF),
 file integrity, AI-based anomaly detection and an operator-friendly TUI.
EOF

# build
fakeroot dpkg-deb --build "$PKGDIR"
OUT="nulleye_${VERSION}_${ARCH}.deb"
mv "$PKGDIR.deb" "$OUT"

echo "Package created: $OUT"
