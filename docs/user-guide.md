# AppImage Manager — User Guide

The application consists of a **daemon** (background service) and a **GUI** (control window). The daemon watches selected folders, discovers AppImage files, and creates application menu shortcuts. The GUI controls the daemon, settings, and installation. Everything runs in user mode (`systemctl --user`).

---

## Table of contents

1. [Requirements](#requirements)
2. [Installation](#installation)
3. [Running from AppImage](#running-from-appimage)
4. [Building from source](#building-from-source)
5. [First-time daemon setup](#first-time-daemon-setup)
6. [GUI overview](#gui-overview)
7. [Installing AppImages (GitHub and URL)](#installing-appimages-github-and-url)
8. [Removing an AppImage](#removing-an-appimage)
9. [Watch directories](#watch-directories)
10. [App settings](#app-settings)
11. [systemctl commands](#systemctl-commands)

---

## Requirements

- **OS:** Linux with systemd (user session), D-Bus, inotify  
- **Build:** C++20, CMake 3.16+, Qt6 (Core, Widgets, DBus, Network), nlohmann-json  

Example (Ubuntu 24.04 / Fedora):

```bash
# Ubuntu / Debian
sudo apt install build-essential cmake qt6-base-dev nlohmann-json3-dev

# Fedora
sudo dnf install gcc-c++ cmake qt6-qtbase-devel json-devel
```

---

## Installation

### Running from AppImage

1. Download `appimage-manager-*.AppImage` from [Releases](https://github.com/wolfDiesel/appimage-manager/releases).
2. Make it executable:  
   `chmod +x appimage-manager-*.AppImage`
3. Run by double-click or from a terminal:  
   `./appimage-manager-*.AppImage`

**On first run** the AppImage will:

- Create config under `~/.config/appimage-manager/`;
- Create a systemd user unit and an application menu entry;
- Start the daemon.

**On every later run** (including after an update):

- The current daemon is stopped;
- The unit and menu entry are updated to point to the **current** AppImage (the one you just launched);
- The daemon is started again.

So to update: download the new AppImage and run it. The old daemon stops, paths switch to the new file, and no manual restart is needed.

If the AppImage Manager binary (the same file) is inside one of the watch directories, it will **not** appear in the app list and cannot be removed via “Remove”.

---

### Building from source

```bash
git clone https://github.com/wolfDiesel/appimage-manager.git
cd appimage-manager
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build -j4
cmake --install build
```

- **`CMAKE_INSTALL_PREFIX`** — install location (`$HOME/.local` for user-only; `/usr/local` for system-wide, then install as root).
- Ensure the bin directory is in your `PATH` (e.g. `~/.local/bin`). If needed, add to `~/.profile` or `~/.bashrc`:  
  `export PATH="$HOME/.local/bin:$PATH"`

---

## First-time daemon setup

If you installed from a package or from source, the daemon does not start by itself. Run once:

```bash
systemctl --user daemon-reload
systemctl --user enable appimage-manager   # autostart on login
systemctl --user start appimage-manager   # start now
```

Check status:

```bash
systemctl --user status appimage-manager
```

Config and data are created on first daemon start under:

```
~/.config/appimage-manager/
```

This holds: watch directory list, AppImage registry, and per-app launch settings (arguments, environment, sandbox).

---

## GUI overview

After the daemon is running, open **AppImage Manager** from the menu or run `appimage-manager-gui`.

### Daemon block

- **Status** — “Daemon: running (N app(s))” or “Daemon: not running”.
- **Start** — start the daemon (`systemctl --user start appimage-manager`).
- **Stop** — stop the daemon.
- **Restart** — restart the daemon.
- **Autostart** — enable/disable daemon autostart on session login (shown as “Enable autostart” / “Disable autostart”).
- **Refresh list** — refresh the application list.
- **App settings…** — launch settings for the selected app in the table.
- **Watch directories…** — add or remove directories watched by the daemon.
- **Install AppImage…** — install an AppImage from GitHub or a direct URL.
- **Remove…** — remove the selected app (file, menu shortcut, registry entry, and settings).

### Application table

List of discovered AppImages: **Name**, **Path**, **Install type**. Sortable by name; selection is kept when the list is refreshed.

If the daemon is not running, the GUI shows “Daemon: not running” and the **Start** button.

---

## Installing AppImages (GitHub and URL)

Use **Install AppImage…** in the GUI.

### GitHub (username/repository)

1. Enter the repo, e.g. `wolfDiesel/appimage-manager`.
2. Click **Install**.
3. The **last 10 releases** are loaded. Pick a release (double-click).
4. A list of **assets** with extension `.appimage` (case-insensitive) is shown. Pick the file (double-click).
5. The file is downloaded into the chosen **watch directory**. The daemon adds it to the registry and creates a menu shortcut.

### Direct URL

1. Paste a direct link to a `.AppImage` file.
2. Click **Install**.
3. The file is saved to the chosen watch directory; the daemon adds it and creates a shortcut.

At least one watch directory must exist (**Watch directories…**).

---

## Removing an AppImage

1. Select the app in the table.
2. Click **Remove…**.
3. Confirm.

Removal does the following:

- Deletes the AppImage **file** from disk;
- Removes the **shortcut** from the application menu (`~/.local/share/applications/`);
- Removes **launch settings** for that app (arguments, environment, sandbox);
- Removes the entry from the daemon **registry**.

The GUI list updates automatically.

---

## Watch directories

The daemon scans only the directories you add. **The directory must exist** when the daemon starts; missing paths are skipped.

Only the **top level** of each directory is scanned (no recursion). A file is treated as an AppImage if its name ends with `.AppImage`; `.part` and `.crdownload` are ignored.

**In the GUI:**

1. **Watch directories…** → **Add…** — choose a directory (e.g. `~/Apps` or `~/Applications`).
2. **Remove** — remove the selected directory from the list.
3. **OK** — the daemon rescans.

Daemon logs:  
`journalctl --user -u appimage-manager -f`

---

## App settings

For the selected app you can set:

- **Arguments** — command-line arguments when launching.
- **Environment** — environment variables (one per line).
- **Sandbox** — none, bwrap, or firejail.

These are saved and used in the menu shortcut. After changing, you may want to click **Refresh list** in the main window.

---

## systemctl commands

| Action           | Command |
|-----------------|--------|
| Start daemon     | `systemctl --user start appimage-manager` |
| Stop             | `systemctl --user stop appimage-manager` |
| Status           | `systemctl --user status appimage-manager` |
| Enable autostart | `systemctl --user enable appimage-manager` |
| Disable autostart| `systemctl --user disable appimage-manager` |
| Restart          | `systemctl --user restart appimage-manager` |

After updating binaries or the unit file, run:  
`systemctl --user daemon-reload`  
and `restart` if needed.
