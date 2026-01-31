# AppImage Manager

Daemon and GUI for discovering and managing AppImages in chosen folders. The daemon watches directories, creates application menu shortcuts; the GUI controls the daemon and launch settings. Runs in user mode (`systemctl --user`).

---

## Quick start

### From AppImage (recommended)

1. Download **appimage-manager-*.AppImage** from [Releases](https://github.com/wolfDiesel/appimage-manager/releases).
2. `chmod +x appimage-manager-*.AppImage`
3. Run: `./appimage-manager-*.AppImage`

On first run the AppImage sets up config, systemd unit, and menu entry, then starts the daemon. On later runs it stops the old daemon, updates paths to the current binary, and restarts — so updating is just “download new AppImage and run it”.

### From source

```bash
git clone https://github.com/wolfDiesel/appimage-manager.git
cd appimage-manager
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build -j4
cmake --install build
```

Then (once):

```bash
systemctl --user daemon-reload
systemctl --user enable appimage-manager
systemctl --user start appimage-manager
```

Run the GUI: **appimage-manager-gui** or **AppImage Manager** from the menu.

---

## Documentation

- **[User guide (English)](docs/user-guide.md)** — installation, GUI, watch directories, install/remove AppImages, App settings, systemctl.
- **[Руководство (русский)](docs/user-guide.ru.md)** — то же на русском.

---

## Requirements

- **OS:** Linux with systemd (user session), D-Bus, inotify  
- **Build:** C++20, CMake 3.16+, Qt6 (Core, Widgets, DBus, Network), nlohmann-json  

---

## License

See [LICENSE](LICENSE) in the repository.
