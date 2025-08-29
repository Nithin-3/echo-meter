# Contributing to echo-meter

Thank you for your interest in contributing to **echo-meter**! Contributions—whether code, documentation, bug reports, or feature requests—are welcome and appreciated.

## How to Contribute

### 1. Reporting Bugs or Suggesting Features

- Open an [issue](https://github.com/Nithin-3/echo-meter/issues) describing the bug or feature you’d like to see.
- Please include clear steps to reproduce bugs, or a concise explanation of your feature idea.

### 2. Submitting Code

1. **Fork** the repository and create your branch from `main`.
2. **Write clear, minimal code** – echo-meter values simplicity and minimalism.
3. **Follow code style conventions** (see below).
4. **Test** your changes before submitting.
5. **Open a Pull Request** and describe your changes.

### 3. Coding Guidelines

- **Language:** echo-meter is written in **C**.
- **Toolkit:** Uses GTK4 and targets Linux (Wayland) environments, especially tiling window managers like Hyprland, i3, or bspwm.
- **BSD is not supported**—please keep development Linux-focused.
- **Write clear, concise, and minimal code.**
- **Document your code** where appropriate.

### 4. Community Guidelines

- Be friendly and welcoming! Newcomers are encouraged to participate.
- See the [Code of Conduct](./CODE_OF_CONDUCT.md) for expected behavior.
- Respect the minimal, no-frills design philosophy of the project.

### 5. Setting Up Your Development Environment

1. Clone your fork:
   ```sh
   git clone https://github.com/your-username/echo-meter.git
   cd echo-meter
   ```
2. Install dependencies:
   - GTK4 development files (`libgtk-4-dev` or similar for your distro)
   - Other dependencies as listed in the README or source
3. Build and run:
   ```sh
   make
   ./echo-meter
   ```
   *(If there are any additional build steps, update here.)*

### 6. License

By contributing you agree that your contributions will be licensed under the [MIT License](./LICENSE) (or the license used by the project).

---

Thank you for helping make echo-meter better!

If you have questions, open an issue or email [nithin3dev@gmail.com](mailto:nithin3dev@gmail.com).
