# Ashley's Fighter Jet — Xbox prototype

A small C adaptation of the HTML5 game made with Ashley. Fly a blue jet through scrolling stars, dodge friendly-looking colorful UFOs, and shoot. Start with 10 bullets; dodging earns one bullet and one point, shooting an alien earns three points. Survive 24 aliens to win (about 32 seconds). Three bonks end the run; Start restarts.

The HTML5 reference is `ZacharyLeahan/ashleyfighterjet`, commit `4ad80b38d9d38fecd79c5dcf823747032a3677a9`. Its title, blue delta jet, navy sky, yellow/orange accents, green UFO pilots, and ammo mechanic guide this adaptation. The reference repository was only read, never modified. This prototype has no Raptor assets or code.

## Setup and build on macOS

```sh
brew install cmake coreutils llvm lld
./scripts/setup-nxdk.sh
./scripts/build.sh
./scripts/test.sh
```

nxdk stays outside this game repository, defaulting to `~/Developer/toolchains/nxdk`. Set `NXDK_DIR` to override. The build stages source in a temporary directory because nxdk Makefiles do not reliably handle spaces in workspace paths. No shell profile changes are needed.

Output: `build/default.xbe`, `build/game.iso`, matching `main.exe` with DWARF information and `main.pdb`, `build.log`, and SHA-256 hashes. Assets are currently drawn in code, so the XISO needs only `default.xbe`. Keep these files together when investigating a particular build. Rebuild after edits before launching. `game.c` contains platform-independent fixed-step gameplay; SDL rendering and input live separately.

## Run in xemu

```sh
brew install --cask xemu
./scripts/run-xemu.py --bootrom /external/path/mcpx_1.0.bin \
  --bios /external/path/homebrew-compatible-bios.bin \
  --hdd /external/path/xbox_hdd.qcow2
```

Supply your own MCPX and BIOS files. The BIOS must support unsigned homebrew; an unmodified retail BIOS will not work in xemu. See the [official required-files documentation](https://xemu.app/docs/required-files/) for requirements and its link to a copyright-free preformatted HDD. This repository does not contain or download firmware.

The launcher uses documented `-config_path`, `-dvd_path`, and `-snapshot` options. It creates a dedicated config, EEPROM, and log under `~/Library/Application Support/AshleyFighterJet/xemu-test`; HDD writes are discarded on exit. It explicitly selects **64 MB**. Your normal xemu config is not used. Use a dedicated test HDD. `--check` validates input files without launching; it does not verify firmware suitability or boot. Environment alternatives: `XBOX_MCPX`, `XBOX_BIOS`, `XBOX_HDD`.

Xbox controls: **Start** to begin/restart, **left stick or D-pad** to move, **A** to shoot. Bind a controller in xemu's Input settings. xemu's keyboard controller can be selected there; its default Start is Return and A is the A key. No controller-injection API is assumed.

## Mac preview

```sh
brew install sdl2
./scripts/preview.sh
```

The same gameplay/rendering code runs on macOS: Return starts/restarts, arrows steer, Space fires; SDL controllers also work. This is a convenient preview, not Xbox validation.

## Pinned dependencies and validation

- nxdk: `14d5ee97e73347c973f1f57b68b79ec08c9e77f2` (`nxdk.lock`); recursive submodules use that commit's pins.
- nxdk SDL2: `2554902f7bbf469449216ee25f88016421271cd8`.
- extract-xiso: `b72e5b60d598ec6df80534cda19cdcd4361aa18c`.
- Tested build tools: LLVM/lld 23.1.2, CMake 4.4.3, coreutils 9.12.
- Installed xemu: 0.8.136, commit `fc24584ce88f0915ad7f04775bb7712c2e3f49ee`.
- Host preview: SDL2 compatibility 2.32.74.
- Development machine: M2 MacBook Air, 24 GB RAM, macOS 26.7.

**Compiled:** minimal SDL program and playable prototype, XBE and XISO generated. **Host tests:** collisions, movement bounds, firing cooldown, dodge rewards, damage/invulnerability, loss, all 24 waves, completion, and restart pass with AddressSanitizer/UndefinedBehaviorSanitizer. **Mac visual check:** title, gameplay, UFOs, game over, restart, and shooting consuming ammo verified. **xemu boot:** verified on 2026-10-04; title screen visibly running at 64 MB using the Xbox’s existing CerBIOS plus Fancy Mouse 0.9.0 `mouse_rev1.bin` and the official HDD image. **xemu gameplay/controller validation:** pending. **Physical Xbox:** untested.

Known limitations: simple placeholder graphics, one enemy type, deterministic waves, no sound, pickups, boss, shop, persistence, or upgrades. This is an early adaptation, not the complete HTML5 game. nxdk emits library/linker warnings; the game sources compile without warnings. Performance and controller behavior on Xbox still need testing. Emulator results will not establish physical Xbox compatibility. The separate ESP-KVM project is untouched.

Next: verify controller input, movement/firing/bonks/restart and level completion in xemu, then improve resemblance to the HTML5 game incrementally. Add GDB only when a concrete issue warrants it; XBDM and a test server are unnecessary for this milestone.

Local installation changes are recorded in [setup notes](docs/setup-notes.md).

References: [nxdk](https://github.com/XboxDev/nxdk), [SDL graphics sample](https://github.com/XboxDev/nxdk/tree/14d5ee97e73347c973f1f57b68b79ec08c9e77f2/samples/sdl), [controller sample](https://github.com/XboxDev/nxdk/tree/14d5ee97e73347c973f1f57b68b79ec08c9e77f2/samples/sdl_gamecontroller), [xemu CLI](https://xemu.app/docs/cli/).

## Verified emulator setup

The firmware blocker was resolved with an existing `C:\Cerbios.bin` from the user’s physical Xbox (SHA-256 `c5e9d940faf66692b56a16f7a7d779445c1c6d9191d7e7e31c1c1e8168da0733`), [Fancy Mouse Boot ROM 0.9.0](https://github.com/SnowyMouse/fancy-mouse-boot-rom/releases/tag/0.9.0) `mouse_rev1.bin`, and [xemu HDD image 1.0](https://github.com/xemu-project/xemu-hdd-image/releases/tag/1.0). This particular combination was verified by booting the game; compatibility with other BIOS versions is not implied. No physical Xbox files were modified.

Local system files are kept outside Git under the user’s application-support folder. To repeat this setup on the configured Mac:

```sh
SYSTEM_FILES="$HOME/Library/Application Support/AshleyFighterJet/system"
./scripts/run-xemu.py --bootrom "$SYSTEM_FILES/mouse_rev1.bin" \
  --bios "$SYSTEM_FILES/Cerbios.bin" --hdd "$SYSTEM_FILES/xbox_hdd.qcow2"
```

The emulator uses its own generated EEPROM; the Xbox’s personal EEPROM and HDD key were not copied. The existing Complex file was labeled BFM and was not used for emulator startup.
