#!/bin/bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
export NXDK_DIR=${NXDK_DIR:-"$HOME/Developer/toolchains/nxdk"}
PIN=$(cat "$ROOT/nxdk.lock")
[ "$(git -C "$NXDK_DIR" rev-parse HEAD)" = "$PIN" ] || { echo 'nxdk commit differs from nxdk.lock' >&2; exit 1; }
BREW_PREFIX=$(brew --prefix)
export PATH="$BREW_PREFIX/opt/llvm/bin:$BREW_PREFIX/opt/lld/bin:$NXDK_DIR/bin:$PATH"
# nxdk Makefiles require a working directory without spaces. Stage a fresh copy.
STAGE=$(mktemp -d /tmp/ashley-xbox-build.XXXXXX)
trap 'rm -rf "$STAGE"' EXIT
cp "$ROOT/Makefile" "$STAGE/"
cp -R "$ROOT/src" "$STAGE/"
mkdir -p "$ROOT/build"
make -C "$STAGE" -j4 2>&1 | tee "$ROOT/build/build.log"
cp "$STAGE/bin/default.xbe" "$STAGE/game.iso" "$STAGE/main.exe" "$ROOT/build/"
for symbols in "$STAGE"/*.pdb; do [ ! -f "$symbols" ] || cp "$symbols" "$ROOT/build/"; done
shasum -a 256 "$ROOT/build/default.xbe" "$ROOT/build/game.iso" "$ROOT/build/main.exe" > "$ROOT/build/SHA256SUMS"
