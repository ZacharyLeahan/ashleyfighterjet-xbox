#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
clang -std=c11 -Wall -Wextra -Werror -DHOST_BUILD -Dmain=game_main $(sdl2-config --cflags) -c src/main.c -o build/controller-main.o
clang -std=c11 -Wall -Wextra -Werror -Isrc $(sdl2-config --cflags) src/game.c src/audio.c src/sound.c tests/controller_test.c build/controller-main.o $(sdl2-config --libs) -o build/controller-test
./build/controller-test
