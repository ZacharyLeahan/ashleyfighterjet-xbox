# Local setup changes (2026-10-04)

Started with an empty game workspace. Initialized a standalone Git repository here; no dependency forks, remote, push, or ESP-KVM changes.

Cloned official XboxDev nxdk recursively outside the workspace and detached it at the pin in `nxdk.lock`. SDK builds generate ignored/untracked build files in its dependency checkout; dependency source was not edited.

Installed Homebrew LLVM/lld 23.1.2 and coreutils 9.12. Homebrew also installed/upgraded their dependencies (xz, z3, gmp). Installed official macOS Xemu.app 0.8.136 in Applications. Checking its version created xemu's normal default config/EEPROM paths; game launches use a separate directory instead.

Updated the existing Homebrew SDL2 compatibility package from 2.32.70 to 2.32.74 for a Mac preview. Homebrew also updated SDL3 to 3.4.18 and its installed dependent ffmpeg from 8.1.2 to 9.0.2 with dependencies. No shell profile was edited.

The minimal compiled program is preserved locally in ignored `build/minimal/`, with its source recoverable in the first local commit. Current prototype artifacts live in `build/`. No proprietary files were downloaded or added.

Visual checks in the Mac preview confirmed the title, UFOs, game-over state, restart, and a shot consuming ammo. A very short key press originally got lost between SDL polling and the fixed simulation step; key/controller press events now stay pending until consumed. Xbox input remains unverified.
