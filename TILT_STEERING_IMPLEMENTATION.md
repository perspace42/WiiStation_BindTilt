# WiiStation Tilt-to-Analog Steering Binding — Implementation Notes

## Why Tilt Differs from a PS1 Analog Stick

### PS1 Analog Stick (DualShock / DualAnalog)
- Each axis is a raw **8-bit unsigned byte** (0–255), center = 128, mapping to a signed range of −128 to +127
- Spring-centered — instantly returns to zero when released
- **Linear**, very low latency, virtually no noise
- Typical per-axis deadzone is 5–10 units (physically built-in)
- Full travel is ~15 mm; games like Crash Team Racing use left-stick X for steering

### Wii Remote Accelerometer / Tilt
- Reports gravity-compensated angles via `wpad->orient.roll` / `wpad->orient.pitch` (degrees)  
- **No spring center** — user must keep the remote level for zero output  
- Bluetooth adds ~10–20 ms latency; libogc applies basic smoothing  
- **Effective steering range** is ±30–45° before the position becomes uncomfortable  
- The full ±180° physical range must map to ±127, but most of it is unusable  

### Key Differences at a Glance

| Dimension | PS1 Analog Stick | Wii Remote Tilt |
|---|---|---|
| Auto-center | Yes (spring) | No — user must level |
| Latency | < 1 frame | ~10–20 ms + BT |
| Noise | Very low | Moderate (hand-shake) |
| Physical range | 15 mm = ±127 | ±180° (usable ≈ ±35°) |
| Ideal curve | Linear | Center deadzone + higher gain |
| Steering axis | Left stick X | Roll (side-tilt) → stickX |

---

## What Changed in the Code

### File: `Gamecube/gc_input/controller-WiimoteNunchuk.c`

1. **Added `TILT_STEERING_AS_ANALOG` enum value** — separate from the pre-existing
   `TILT_AS_ANALOG` mode so existing Wiimote profiles are unaffected.
2. **Correct axis mapping** — roll (side-tilt) → `stickX`; pitch disabled (set to 0)
   so pure left/right steering works as expected in kart games.
3. **Center deadzone** (`TILT_DEADZONE_DEG = 5.0°`) — angles within this band report
   zero, preventing drift from hand-shake.
4. **Non-linear sensitivity curve** (square-root) — small tilts produce proportionally
   larger output than a purely linear map, matching the narrow useful tilt range.
5. **Exposed in both analog-source tables** — `analog_sources_wm[]` (Wiimote-only)
   and `analog_sources_wmn[]` (Wiimote + Nunchuk) so the in-game menu can select it.
6. **WPAD data format** updated so `WPAD_DATA_ACCEL` is requested when this mode is active.

### Profile Persistence
The existing `load_configurations` / `save_configurations` in `PlugPAD.c` store
`analogL` / `analogR` as integer indices into the `analog_sources[]` array.
Adding the new entry to the array is sufficient — no changes to the save/load code
are needed.

---

## Tuning the Feel

Two `#define` constants at the top of `controller-WiimoteNunchuk.c` control the feel:

| Constant | Default | Effect |
|---|---|---|
| `TILT_DEADZONE_DEG_DEFAULT` | `5.0f` | Degrees of roll that map to zero output |
| `TILT_MAX_DEG_DEFAULT` | `35.0f` | Degrees of roll that produce full (±127) output |

These are now defined in `controller.h` and used as initial values for **per-profile**
fields `tiltDeadzone` and `tiltMaxAngle` stored inside `controller_config_t`.

---

## Per-Profile Deadzone & Max Angle (v2 Config Format)

As of config version 2, each of the four save slots stores its own `tiltDeadzone`
and `tiltMaxAngle` values. This lets you tune differently for different games:

| Game example | Suggested tiltDeadzone | Suggested tiltMaxAngle |
|---|---|---|
| Crash Team Racing | 5° | 30–35° |
| Ridge Racer / NFS | 4° | 22–28° |
| Gran Turismo 1/2 | 6° | 35–40° |
| Formula 1 games | 5° | 35° |

### In-game UI controls (Wiimote/Wiimote+Nunchuk only)

In **Settings → Configure Buttons**, two new rows appear below the Sensitivity control:

```
[ − ]  DZ: 5.0   [ + ]       ← deadzone in degrees (±0.5° steps, 0–15°)
[ − ]  Max: 35   [ + ]       ← max angle in degrees (±1° steps, 10–60°)
```

Press **Save** to persist the current values to the selected slot.
The controls are hidden for non-Wiimote controller types (GC, Classic, Pro, HID)
so the existing layout is completely unchanged for those pads.

### Binary format change

Two extra `float` fields (8 bytes per slot × 4 slots = 32 extra bytes per controller
file) are appended after `sensitivity` and before `fastf`.  
`CONTROLLER_CONFIG_VERSION` has been bumped **1 → 2** — old saves are discarded on
load and replaced with defaults (the emulator always fell back gracefully).
