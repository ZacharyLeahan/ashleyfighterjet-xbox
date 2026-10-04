#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
clang -std=c11 -Wall -Wextra -Werror -DHOST_BUILD $(sdl2-config --cflags) src/*.c $(sdl2-config --libs) -o build/ashley-preview
exec ./build/ashley-preview
