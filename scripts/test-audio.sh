#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p build
clang -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined -g -Isrc src/sound.c tests/sound_test.c -o build/sound-test
./build/sound-test
clang -std=c11 -Wall -Wextra -Werror -fsanitize=undefined -g -Isrc $(sdl2-config --cflags) src/sound.c src/audio.c tests/audio_test.c $(sdl2-config --libs) -o build/audio-test
./build/audio-test
