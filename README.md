# QuickJot

A lightweight, modern, offline notes application for Linux, built with **C++17** and **Qt 6 Widgets**. No accounts, no cloud, no subscriptions, no telemetry — just your notes, saved locally as JSON.

## Features

- **Create, edit, delete** notes with a single click.
- **Instant search** across both titles and full note content.
- **Automatic saving** — every keystroke is persisted (debounced) to disk.
- **Local JSON storage** — plain, human-readable, easy to back up.
- **Dark mode UI** — clean, distraction-free, custom stylesheet.
- **Completely offline** — no network code anywhere in the binary.

## Screenshot

_(Add `docs/screenshot.png` after first run if you want a preview in the README.)_

## Building from source

### Dependencies

- A C++17 compiler (GCC 9+ or Clang 10+)
- CMake **3.16** or newer
- Qt **6** (Widgets module)
- For AppImage packaging: `linuxdeploy` + `linuxdeploy-plugin-qt` + `appimagetool` (auto-downloaded by the script)

#### CachyOS / Arch / Manjaro

```bash
sudo pacman -S --needed base-devel cmake qt6-base
