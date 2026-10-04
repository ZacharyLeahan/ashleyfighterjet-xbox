#!/bin/bash
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
SDK=${NXDK_DIR:-"$HOME/Developer/toolchains/nxdk"}
PIN=$(cat "$ROOT/nxdk.lock")
if [ -e "$SDK" ]; then
    [ "$(git -C "$SDK" rev-parse HEAD)" = "$PIN" ] || { echo 'Existing SDK differs from pin; use a new NXDK_DIR.' >&2; exit 1; }
else
    mkdir -p "$(dirname "$SDK")"
    git clone https://github.com/XboxDev/nxdk.git "$SDK"
    git -C "$SDK" checkout --detach "$PIN"
fi
git -C "$SDK" submodule update --init --recursive
