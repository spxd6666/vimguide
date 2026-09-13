#!/usr/bin/env bash
# Builds a source tarball and packages vimguide as an .rpm using rpmbuild.
# Requires rpmbuild (rpm-based distro: `sudo dnf install rpm-build` /
# `sudo zypper install rpm-build`; on Debian/Ubuntu: `sudo apt install rpm`).
# Run from the project root: ./scripts/build-rpm.sh
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$PROJECT_ROOT"

if ! command -v rpmbuild >/dev/null 2>&1; then
    echo "error: rpmbuild not found." >&2
    echo "Install it first, e.g.:" >&2
    echo "  Fedora/RHEL/openSUSE : sudo dnf install rpm-build  (or: sudo zypper install rpm-build)" >&2
    echo "  Debian/Ubuntu        : sudo apt install rpm" >&2
    exit 1
fi

VERSION="$(grep -m1 '^Version:' packaging/rpm/vimguide.spec | awk '{print $2}')"
NAME="vimguide"
WORKDIR="$(mktemp -d)"
TARBALL_DIR="$WORKDIR/${NAME}-${VERSION}"

echo "==> Preparing source tree for tarball"
mkdir -p "$TARBALL_DIR"
cp -r src man Makefile README.md LICENSE "$TARBALL_DIR/"

mkdir -p dist
tar -C "$WORKDIR" -czf "dist/${NAME}-${VERSION}.tar.gz" "${NAME}-${VERSION}"

echo "==> Setting up rpmbuild tree"
RPMBUILD_ROOT="$WORKDIR/rpmbuild"
mkdir -p "$RPMBUILD_ROOT"/{SOURCES,SPECS,BUILD,RPMS,SRPMS}
cp "dist/${NAME}-${VERSION}.tar.gz" "$RPMBUILD_ROOT/SOURCES/"
cp packaging/rpm/vimguide.spec "$RPMBUILD_ROOT/SPECS/"

echo "==> Running rpmbuild"
rpmbuild --define "_topdir $RPMBUILD_ROOT" -ba "$RPMBUILD_ROOT/SPECS/vimguide.spec"

echo "==> Collecting output rpm(s)"
find "$RPMBUILD_ROOT/RPMS" -name '*.rpm' -exec cp {} dist/ \;
find "$RPMBUILD_ROOT/SRPMS" -name '*.rpm' -exec cp {} dist/ \;

rm -rf "$WORKDIR"
echo "==> Done. Packages placed in dist/:"
ls -1 dist/*.rpm
