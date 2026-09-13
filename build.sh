#!/usr/bin/env bash
# Convenience wrapper: builds the binary, then the .deb, then the .rpm
# (rpm step is skipped with a warning if rpmbuild isn't installed).
set -euo pipefail
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$PROJECT_ROOT"

./scripts/build-deb.sh

if command -v rpmbuild >/dev/null 2>&1; then
    ./scripts/build-rpm.sh
else
    echo
    echo "Skipping .rpm build: rpmbuild is not installed on this machine."
    echo "Install it (see scripts/build-rpm.sh header) and re-run ./scripts/build-rpm.sh"
fi

echo
echo "==> Build artifacts:"
ls -1 dist/ 2>/dev/null || true
