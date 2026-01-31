# AppImage Manager

Демон и GUI для учёта и управления AppImage в выбранных папках. Демон следит за каталогами, создаёт ярлыки в меню приложений; GUI управляет демоном и настройками запуска. Режим пользователя (`systemctl --user`).

**Сборка:** см. [docs/user-guide.md](docs/user-guide.md) (зависимости, `cmake -B build`, `cmake --build`, `cmake --install`).

**Установка и первый запуск:** там же — `systemctl --user daemon-reload`, `enable`, `start`, запуск `appimage-manager-gui`.
