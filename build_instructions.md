# WiiStation — Build & Wii U Port Instructions

## Prerequisites

WiiStation is built using the **devkitPro** toolchain targeting the Wii (PowerPC).
You need these components installed and available on your `PATH`:

| Component | Version | Source |
|---|---|---|
| devkitPPC | r41-2 | https://wii.leseratte10.de/devkitPro/ |
| libogc2 | up to commit `7456c4ab` | Included in `lightrec+Libogc2.zip` |
| SDL (Wii port) | — | `libSDL.a` in repo root |
| GNU Lightning + Lightrec | — | `lightrec+Libogc2.zip` in repo root |

> **Tip:** The repo includes pre-compiled zip archives (`lightrec+Libogc2.zip`,
> `libSDL.a`, `LightrecByPPC29Libogc1.6.zip`) containing all Wii-specific libraries.
> Extract them to your devkitPro environment before building.

---

## Setting Up the Build Environment

### 1. Install devkitPro on Linux / macOS / WSL

```bash
# Install the devkitPro pacman helper
curl -L https://github.com/devkitPro/pacman/releases/latest/download/devkitpro-pacman-installer.sh | bash

# Install the Wii toolchain packages
dkp-pacman -S devkitPPC libogc wii-dev
```

### 2. Install devkitPro on Windows (native)

Download the installer from https://devkitpro.org/wiki/Getting_Started and follow
the Windows instructions. Install the **Wii** component group.

> **Recommended:** Use WSL 2 (Windows Subsystem for Linux) for the smoothest
> build experience. The toolchain works natively on Linux without path issues.

### 3. Extract the pre-built libraries

```bash
# Inside the WiiStation_BindTilt repo root:
unzip lightrec+Libogc2.zip
# Copy / overwrite the extracted files into $DEVKITPRO/portlibs/wii/
# (the zip contains portlibs-layout-compatible paths)
```

### 4. Set environment variables

```bash
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC=$DEVKITPRO/devkitPPC
export PATH=$DEVKITPPC/bin:$PATH
```

---

## Building the Project

Run from the **repo root** (`WiiStation_BindTilt/`):

```bash
# Release build (Wii target, uses Lightrec dynarec)
make -f Gamecube/Makefile_Wii

# If you want the older PPC dynarec variant:
# make -f Gamecube/Makefile_Wii_Release
```

Successful output ends with something like:

```
linking ... boot.elf
built ... boot.dol
```

The deliverable is **`boot.dol`** in the repo root (or `Gamecube/release/`).

### Cleaning

```bash
make -f Gamecube/Makefile_Wii clean
```

---

## Installing on a Real Wii (SD / USB)

1. Format your SD card as FAT32.
2. Place a Homebrew Channel boot file at:
   ```
   sd:/apps/WiiStation/boot.dol
   sd:/apps/WiiStation/meta.xml   ← optional, describe the app
   ```
3. Create the data directory:
   ```
   sd:/wiisxrx/isos/     ← put PS1 game images here (.bin/.cue, .iso, .chd, .pbp)
   sd:/wiisxrx/bios/     ← put SCPH-xxxx.bin BIOS here (optional but recommended)
   sd:/wiisxrx/fonts/chs.dat   ← required font file (included in original releases)
   ```
4. Launch via **Homebrew Channel** on the Wii.

---

## Porting to Wii U (vWii Mode)

The Wii U contains a full virtual Wii (vWii) core. WiiStation runs on vWii **without
recompilation** — the same `boot.dol` used on a real Wii works identically.

### Method 1 — Homebrew Launcher (recommended)

1. **Exploit the Wii U** using an existing Wii U browser or Haxchi exploit:
   - Follow https://wiiu.hacks.guide for the current recommended method.
   - Install the **Homebrew Launcher** channel.

2. **Enable vWii** from the Wii U menu and access the **Wii System Menu**.

3. Install the **Homebrew Channel** inside vWii:
   - From the Homebrew Launcher, launch `homebrew_channel_installer.elf`
     (available from https://wiibrew.org/wiki/Homebrew_Channel).

4. Copy your WiiStation files to the SD card exactly as described in
   "Installing on a Real Wii" above. The Wii U reads the same SD slot in vWii mode.

5. Launch WiiStation from the vWii Homebrew Channel.

### Method 2 — Wii U Forwarder Channel

Install a Wii U forwarder `.wup` that launches vWii homebrew directly from the
Wii U menu without entering vWii first.  Tools like **WUHB Forwarder** or
**WiiVC Injector** can create this from your `boot.dol`.

### Known vWii Differences

| Feature | Real Wii | vWii on Wii U |
|---|---|---|
| Wiimote tilt/accel | ✓ Full support | ✓ Full support |
| Wii U GamePad | ✗ Not available | ✓ Supported (mapped as Wiimote alt) |
| Wii U Pro Controller | ✗ Not available | ✓ Works via BT stack |
| SD card path | `sd:/` | `sd:/` (same) |
| USB storage | ✓ | ✓ (some adapters may differ) |
| Performance | Baseline | Identical to real Wii |

> **Tilt Steering on vWii:** Wii Remotes pair to the Wii U's Bluetooth exactly as on
> a real Wii, so the new **Tilt Steering** analog source works without any changes.

---

## Enabling the Tilt Steering Binding (New Feature)

1. Launch WiiStation and load a game.
2. Press **Home** (or your configured menu button) to open the emulator menu.
3. Go to **Settings → Configure Buttons**.
4. Your current controller (Wiimote or Wiimote+Nunchuk) should be shown.
5. Navigate to **Analog Stick L** and press A/click to cycle the source.
6. Select **"Tilt Steering"**.
7. *(Optional)* Adjust **Sensitivity** with the `−` / `+` buttons.
8. Press **Save** to store the mapping to a configuration slot.

### Tuning Feel

Edit these two `#define`s near the top of
`Gamecube/gc_input/controller-WiimoteNunchuk.c` and rebuild:

```c
#define TILT_DEADZONE_DEG  5.0f   // angle (degrees) ignored near centre
#define TILT_MAX_DEG      35.0f   // angle (degrees) mapped to full ±127 output
```

| If the kart… | Adjust… |
|---|---|
| Drifts when remote is level | Increase `TILT_DEADZONE_DEG` |
| Feels sluggish to steer | Decrease `TILT_MAX_DEG` |
| Turns too sharply on small tilts | Increase `TILT_MAX_DEG` |
| Still drifts after deadzone change | Decrease `TILT_DEADZONE_DEG` |

---

## Troubleshooting

| Symptom | Likely Cause | Fix |
|---|---|---|
| `boot.dol` not found by HBC | Wrong directory structure | Ensure `sd:/apps/WiiStation/boot.dol` |
| Font/text missing | `chs.dat` absent | Copy `fonts/chs.dat` to `sd:/wiisxrx/fonts/` |
| Game won't load | BIOS missing | Place a `BIOS/SCPH-XXXX.bin` in `sd:/wiisxrx/bios/` |
| Tilt Steering not in menu | Old saved config loaded | Switch to Default config in Configure Buttons |
| Build fails: `libogc not found` | Libraries not extracted | Unzip `lightrec+Libogc2.zip` into devkitPro |
| Wiimote not responding in vWii | Pairing lost | Re-sync Wiimote using red button on Wii U back |
