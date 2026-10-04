#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
clang -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g -Isrc src/game.c tests/game_test.c -o build/game-test
./build/game-test
