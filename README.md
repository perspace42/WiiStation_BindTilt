# WiiStation

WiiStation is a Sony PlayStation 1 (PS1/PSX/PSone) emulator for the Nintendo Wii and Wii U, forked from the original WiiSX-RX. This fork adds Wiimote Tilt Steering specifically optimized for kart and racing games, along with other usability improvements.

---

## Description

WiiStation allows you to play PS1 games on your Wii/Wii U. This project builds upon PCSX-ReARMed and PCSX-Reloaded ports, improving compatibility, audio, performance (via Lightrec), and controller support. This specific "BindTilt" fork introduces a customized steering scheme utilizing the Wiimote's roll axis for precise, responsive racing controls with configurable deadzones and sensitivity curves, replacing standard analog stick input with natural motion control. Additionally, it features per-game controller mapping and persistent default settings.

---

## Requirements

- Nintendo Wii or Wii U (vWii mode) with Homebrew capability
- SD card or USB drive for loading games and saving settings
- Supported controllers (Wiimote, Nunchuk, Classic Controller, GameCube Controller, or supported USB HID controllers)

---

## Features

### ✨ Wiimote Tilt Steering

A **Tilt Steering** analog-source binding designed for kart and racing games (e.g., Crash Team Racing, Gran Turismo, Ridge Racer).

Hold the Wii Remote horizontally and tilt it **side-to-side (roll)** — just like a steering wheel. The rotation angle is translated into the PS1 left analog stick's X-axis with a **centre deadzone** and a **square-root response curve** for fine control near straight-ahead.

| Wii Remote motion | PS1 output |
|---|---|
| Held level | Stick centred — no steering |
| Tilt right | Steer right (positive X) |
| Tilt left | Steer left (negative X) |

**Why this is better than the built-in "Tilt" mode for racing:**

| | Legacy Tilt | **Tilt Steering (new)** |
|---|---|---|
| Steering axis | Pitch → X (nose up/down) | **Roll → X** (natural wheel feel) |
| Centre dead zone | None — drifts constantly | ±2.5° by default — holds centre |
| Response curve | Linear | Square-root — fine control near centre |
| Pitch axis | Active | Disabled — pure left/right only |
| Tunable per game | No (rebuild required) | **Yes — saved per config slot** |

### Per-Profile Tuning (in-game, no rebuild needed)

When a Wiimote or Wiimote+Nunchuk is active, two extra rows appear in the **Configure Buttons** screen below the Sensitivity control:

``
[ − ]  DZ: 2.5   [ + ]      ← Centre dead zone in degrees
[ − ]  Max: 45   [ + ]      ← Max steering angle in degrees
``

| Control | Step | Range | What it does |
|---|---|---|---|
| **DZ** (Deadzone) | ±0.5° | 0° – 15° | Degrees of roll ignored near level. Raise if the kart drifts when the remote is held flat. |
| **Max** (Max angle) | ±1° | 10° – 60° | Degrees of roll that produce full lock (±127). Lower = snappier; raise = gentler. |

These controls are hidden for non-Wiimote pad types (GameCube, Classic, Pro, HID).

#### Recommended starting points

| Game | DZ | Max |
|---|---|---|
| Crash Team Racing | 2.5° | 32° |
| Ridge Racer / Need for Speed | 2.5° | 25° |
| Gran Turismo 1 / 2 | 2.5° | 38° |
| Formula 1 / Rally | 2.5° | 35° |

### Additional Fixes and Improvements in This Fork
- **Per-Game Controller Bindings**: Button mappings and tilt settings are saved to and loaded from per-game specific config files automatically when a game is loaded, superseding the default global configurations.
- **Settings Persistence**: General, Video, Input, and Audio settings (e.g., PSX Controller Type, Disable Rumble, Show FPS) now automatically save to and load from the SD card.
- **Cleaned File Browser**: The file browser now actively filters out .cue files, displaying only .ccd, .iso, .img, etc. to reduce clutter.
- **Exit Crash Fix**: Fixed a system crash that occurred when exiting to the loader while a game was running by cleanly shutting down audio/video plugins before tearing down the HID interface.

### General WiiStation Features
- HID controllers support (based on Nintendont configurations).
- PCSX-ReARMed CDROM and CDRISO compatibility improvements.
- CDDA (Compact Disc Digital Audio) tracks & multi-tracks support.
- PCSX-ReARMed timer system timings emulation.
- DFSound module with SDL for greatly improved audio quality.
- Lightrec PSX dynamic recompiler for massive speed/performance boosts.
- Video plugins: P.E.Op.S. Soft GPU (faster) or gpulib + DFXVideo (more compatible).
- 240p and Interlace (480i) mode support.
- PS1 Lightguns support (Namco GunCon & Konami Justifier via Wiimote IR).
- Experimental PS1 Mouse support via Wiimote IR.
- PS1 Multitap support (up to 8 players).
- Support for BIN+CUE, ISO, IMG, eboot PBP, and CHD v1-v5 formats.
- Multiple language support.

---

## Installation

Once you have successfully built the project (or downloaded a release), you need to place the compiled .dol file on your SD card so it can be launched via the Homebrew Channel on your Wii or vWii (Wii U). 

*(Note: The WiiSXRX_debug.elf file is only needed for developers using a USB Gecko debugger or similar development setups. Regular users can ignore or delete it.)*

**For a vWii running via Aroma on the Wii U:**
1. Copy the compiled Gamecube/WiiSXRX_debug.dol to your SD card, renaming it to oot.dol.
2. Place it in the following folder structure:
   `ash
   sd:/apps/wiisxrx/boot.dol
   `
3. Copy the meta.xml and icon.png files from the Gamecube/ folder and place them alongside oot.dol so it shows up with the custom icon and description in the Homebrew Channel.
4. Boot into your vWii, launch the Homebrew Channel, and select WiiStation!

See [uild_instructions.md](build_instructions.md) for the full build guide and Podman/Docker instructions.  
See [TILT_STEERING_IMPLEMENTATION.md](TILT_STEERING_IMPLEMENTATION.md) for a technical deep-dive on tilt vs. analog stick mechanics and the full code architecture.

---

## Usage

### Enabling Tilt Steering

1. Launch a game → open the emulator menu → **Settings → Configure Buttons**
2. Find **Analog Stick L** and press A to cycle the source until it reads **"Stick"** (Tilt Steering mode).
3. *(Optional)* Adjust **DZ** and **Max**.
4. Press **Save** to store the mapping to a config slot (1–4) for the specific game.

---

## Configuration

*Config format note: The save format was bumped from version 1 → 2 to store the float settings for tilt control. Any existing version-1 .cfg save files will be ignored on first load and replaced with defaults. You will need to re-save your button layouts once after updating.*

---

## License

This project is licensed under the GPL-2.0 License. See the repository for full license details.

## Acknowledgements

- WiiStation (formerly WiiSXRX_2022) - developed by xjsxjs197
- 240p/Lightgun/Mouse/Multitap support, some fixes by Jokippo
- WiiStation icon - made by Dakangel (high quality logo made by saulfabreg)
- WiiSX-RX fork - developed by NiuuS
- WiiSX-R fork - developed by Mystro256
- PCSX-Revolution - developed by Firnis
- WiiSX - developed by emu_kidid, tehpola, sepp256
- PCSX-ReARMed - developed by notaz, libretro
- libOGC2 - developed by Extrems
- Lightrec - developed by pcercuei
- libCHDr - developed by MAME Team and rtissera
