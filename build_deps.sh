#!/usr/bin/env bash
# build_deps.sh — Build all WiiStation source dependencies for Wii/PowerPC.
#
# Run this AFTER setting up devkitPPC r41-2 and placing the files from
# lightrec+Libogc2.zip into /opt/devkitpro.
#
# Usage:
#   cd /src/WiiStation_BindTilt
#   bash build_deps.sh
#
set -e
set -u
set -o pipefail

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"

# ── 0. Prereq check ──────────────────────────────────────────────────────────
if [ -z "${DEVKITPPC:-}" ] || [ -z "${DEVKITPRO:-}" ]; then
    echo "ERROR: DEVKITPPC and DEVKITPRO must be set."
    exit 1
fi

echo "=== Building source dependencies ==="

# ── 1. zlib ──────────────────────────────────────────────────────────────────
ZLIB_DIR="$REPO_ROOT/deps/libchdr/deps/zlib-1.3.1"
echo ""
echo "=== Building zlib ==="
cd "$ZLIB_DIR"
make -f Makefile.wii clean 2>/dev/null || true
make -f Makefile.wii
cd "$REPO_ROOT"

# ── 2. libchdr ───────────────────────────────────────────────────────────────
LIBCHDR_DIR="$REPO_ROOT/deps/libchdr"
echo ""
echo "=== Building libchdr ==="
cd "$LIBCHDR_DIR"
make -f Makefile.wii clean 2>/dev/null || true
make -f Makefile.wii
cd "$REPO_ROOT"

# ── 3. opengx ────────────────────────────────────────────────────────────────
OPENGX_DIR="$REPO_ROOT/deps/opengx"
echo ""
echo "=== Building opengx ==="
cd "$OPENGX_DIR"
make -f Makefile clean 2>/dev/null || true
make -f Makefile
cd "$REPO_ROOT"

echo ""
echo "All deps built successfully."
echo ""
echo "Next step:"
echo "  cd Gamecube && make -f Makefile_Wii"
