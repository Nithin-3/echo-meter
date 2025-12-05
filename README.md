# echo-meter

A lightweight on-screen display (OSD) and controller for brightness, volume, and microphone, written in C for Linux Wayland compositors (e.g., Hyprland). Offers both reading and adjustment of system settings, with a secure root helper for brightness control.

---
## Screen Recording

![Demo](assetsR/recording-2025-12-05_12-52-22.mp4)

## Features

- Written in C for maximum performance and minimal dependencies
- Directly reads and writes brightness, volume, and microphone levels
- Secure root helper included for writing brightness (only brightness requires root privileges)
- On-screen display for:
  - Brightness (with adjustment)
  - Volume (with adjustment)
  - Microphone (with adjustment)
  - Caps Lock, Num Lock, and Mute states
- Designed for modern Linux desktops using Wayland (tested on Hyprland)
- Perfect for tiling window managers lacking native OSD/controls

---

## Why?

Most tiling window managers on Wayland lack native OSD or controls for volume, brightness, microphone, or lock states. echo-meter fills this gap, offering both user feedback and real control, with a focus on safety (root helper only for brightness write).

---

## Installation

### Dependencies

- `gtk4`
- `gtk4-layer-shell`
- C compiler (`gcc`, `clang`)

### Build

```sh
git clone https://github.com/Nithin-3/echo-meter.git
cd echo-meter
make
```

### Run

```sh
./echo-meter aud
```

### Install (system-wide)

You can install echo-meter system-wide using:

```sh
sudo make install
```

This will copy the binaries and any necessary files to appropriate system directories for all users.

**Note:**  
`echolis` is a listener for echo-meter; run it as yourself as a daemon after installation.

---

## Usage

echo-meter [aud|mic|bri] (+|-) [0-100]

- `aud` – Control output audio volume
- `mic` – Control microphone volume
- `bri` – Control screen brightness

### Examples
- Increase brightness by the configured step:
  ```sh
  echo-meter bri +
  ```
- Add volume to 35:
  ```sh
  echo-meter aud + 35
  ```
- Decrease microphone volume:
  ```sh
  echo-meter mic -
  ```

---

## Configuration

Configuration files are loaded from:
- System-wide: `/usr/share/echo-meter`
- User: `$HOME/.config/echo-meter`

You can override system defaults by copying and editing files in your user config directory.

### Files

- `config.json` – Main configuration
- `style.css` – Custom CSS for OSD appearance

### Example `config.json`

```json
{
  "orientation": "horizontal",           // default: "horizontal"
  "invert-direction": false,             // default: false

  "window_position": {
    // Only use either x/y or vertical+horizontal+margin
    // If x and y are set, `has_explicit_pos = true`
    "x": 40,                              // custom: overrides alignment
    "y": 30,                              // custom: overrides alignment

    "vertical": "top",                    // default: "top" (if x/y missing)
    "horizontal": "center",               // default: "center" (if x/y missing)
    "margin": 50                          // default: 0 (only if align used)
  },

  "timeout": 5,                        // seconds before OSD disappears

  "icon": {
    "sound": "🔊",                         // icon for sound
    "mute": "🔇",                          // icon for mute
    "brightness": "🌞",                    // icon for brightness
    "mic": "🎤",                           // icon for mic
    "mic_off": "🙊"                        // icon for mic off
  },

  "system_info": {
    "volume_step": 5,                    // change step for volume
    "brightness_step": 10,               // change step for brightness
    "mic_step": 3                        // change step for mic
  }
}
```

- All fields are optional; defaults will be used if omitted.
- For positioning, use either `x`/`y` or alignment (`vertical`, `horizontal`, `margin`).

---

## Styling

Customize the appearance with `style.css` (see example in the repo or create your own).

---

## Security

- Only brightness writes require root, handled securely via a helper binary.

---

## License

[MIT](LICENSE)
