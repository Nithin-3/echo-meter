# echo-meter
Try to mimic function key
A lightweight GTK4 layer-shell application that displays always-on-top real-time audio, brightness, and microphone level indicators.

---

## Features

- Always-on-top overlay using GTK4 and `gtk4-layer-shell`
- Shows live system audio volume, screen brightness, and microphone levels
- Configurable via JSON and CSS files in `~/.config/echo-meter` or system-wide `/usr/share/echo-meter`
- Designed for Linux desktops with modern compositors (e.g., Wayland)
- Fast, minimal, and unobtrusive

---

## Requirements

GTK4, gtk4-layer-shell, and json-glib libraries.

---

## Installation

### Arch Linux

You can install the dependencies on Arch Linux using:

```bash
sudo pacman -S gtk4 gtk4-layer-shell json-glib
```

A package may be available in the AUR. To build from source, ensure the dependencies above are installed.

---

### Debian/Ubuntu

```bash
sudo apt install libgtk-4-dev libgtk-layer-shell-0.1-dev libjson-glib-dev
```

---

## Build

To build the project, simply run:

```bash
make
```

---

## Usage

After building, you can run the application with:

```bash
make run
```

Or directly:

```bash
./echo-meter
```

---

## Installation (System-wide)

To install the binary and assets:

```bash
sudo make install
```

This will:
- Install the binary to `/usr/local/bin/echo-meter`
- Copy assets to `/usr/share/echo-meter`

You can then run `echo-meter` from anywhere.

---

## Uninstall

To remove the installed files and configuration:

```bash
sudo make uninstall
```

---

## Configuration

echo-meter is configurable via files in `~/.config/echo-meter` (user-specific) or `/usr/share/echo-meter` (system-wide):

### Example `config.json`

```json
{
  "orientation": "horizontal",
  "invert-direction": true, // default: false
  "window_position": {
    "x": 40,
    "y": 30, // x, y have high priority if set
    "vertical": "top",
    "horizontal": "center",
    "margin": 50
  }
}
```

- `orientation`: `"horizontal"` or `"vertical"` layout for indicators.
- `invert-direction`: If true, reverses the indicator direction.
- `window_position`: Controls the window's placement. If `x` and `y` are set, they take priority. Otherwise, the overlay is positioned using `vertical`, `horizontal`, and `margin`.

### Example `style.css`

```css
#status-bar {
    color: #019606;           /* Text color */
    font-weight: bold;
    font-size: 14px;
    border-radius: 0;
}

#status-bar trough {
    background-color: #222222; /* Optional: dark grey trough */
    border-radius: 0;
    min-height: 20px;
}

#status-bar trough progress {
    background-color: #019606;
    border-radius: 0;
    min-height: 20px;
}
#progress-label {
    color: rgba(1, 150, 6, 1);
    font-weight: bold;
    text-shadow: 0 0 5px #000000;
}
```

- Customize colors, fonts, and layout by editing style.css.
- You can copy the default configuration and CSS from `/usr/share/echo-meter` after install:

```bash
mkdir -p ~/.config/echo-meter
cp /usr/share/echo-meter/config.json ~/.config/echo-meter/
cp /usr/share/echo-meter/style.css ~/.config/echo-meter/
```

Edit these files to customize indicator order, colors, position, and more.

---

## Known Issues / Limitations

- Only supports modern Linux desktops (e.g., Wayland compositors).
- X11 is not supported.
- Features may vary depending on compositor and hardware support.

---

## Contributing

Contributions, bug reports, and suggestions are welcome!
- Open an issue or pull request on GitHub.
- Please follow the existing code style and include a description of your changes.

---

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.

---

## Credits

- Built using [GTK4](https://www.gtk.org/) and [gtk4-layer-shell](https://github.com/wmww/gtk-layer-shell)
- Inspired by other minimal overlay tools for Linux

---
