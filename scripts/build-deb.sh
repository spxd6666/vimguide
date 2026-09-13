#!/usr/bin/env bash
# Builds vimguide and packages it as a .deb using dpkg-deb.
# Run from the project root: ./scripts/build-deb.sh
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

VERSION="$(grep -m1 '^Version:' packaging/debian/DEBIAN/control | awk '{print $2}')"
ARCH="$(dpkg --print-architecture 2>/dev/null || echo amd64)"
STAGE="$(mktemp -d)"
PKGNAME="vimguide_${VERSION}_${ARCH}"

echo "==> Compiling vimguide"
make clean >/dev/null 2>&1 || true
make

echo "==> Staging package tree in $STAGE"
mkdir -p "$STAGE/DEBIAN"
mkdir -p "$STAGE/usr/bin"
mkdir -p "$STAGE/usr/share/man/man1"
mkdir -p "$STAGE/usr/share/doc/vimguide"

cp packaging/debian/DEBIAN/control "$STAGE/DEBIAN/control"
sed -i "s/^Architecture:.*/Architecture: ${ARCH}/" "$STAGE/DEBIAN/control"

install -m 0755 vimguide "$STAGE/usr/bin/vimguide"
install -m 0644 man/vimguide.1 "$STAGE/usr/share/man/man1/vimguide.1"
gzip -n -9 "$STAGE/usr/share/man/man1/vimguide.1"
[ -f README.md ] && cp README.md "$STAGE/usr/share/doc/vimguide/README.md"

# compute installed size in KB for the control file (best-effort, non-fatal)
SIZE_KB="$(du -sk "$STAGE/usr" | cut -f1)"
{ echo "Installed-Size: ${SIZE_KB}"; cat "$STAGE/DEBIAN/control"; } > "$STAGE/DEBIAN/control.tmp"
mv "$STAGE/DEBIAN/control.tmp" "$STAGE/DEBIAN/control"

mkdir -p dist
echo "==> Building .deb"
dpkg-deb --build --root-owner-group "$STAGE" "dist/${PKGNAME}.deb"

echo "==> Verifying package"
dpkg-deb --info "dist/${PKGNAME}.deb"
dpkg-deb --contents "dist/${PKGNAME}.deb"

rm -rf "$STAGE"
echo "==> Done: dist/${PKGNAME}.deb"
echo "Install with: sudo dpkg -i dist/${PKGNAME}.deb"
