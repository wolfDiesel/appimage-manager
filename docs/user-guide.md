# AppImage Manager — руководство пользователя

Приложение состоит из демона (сервис) и GUI. Демон следит за выбранными папками, находит AppImage-файлы и создаёт ярлыки в меню приложений. GUI управляет демоном и настройками. Всё работает в режиме пользователя (`systemctl --user`).

---

## Зависимости

- Linux с поддержкой systemd (user session), D-Bus, inotify
- Для сборки: C++20, CMake 3.16+, Qt6 (Core, Widgets, DBus, Network), nlohmann-json

Пример установки зависимостей (Ubuntu 24.04):

```bash
sudo apt install build-essential cmake qt6-base-dev nlohmann-json3-dev
```

---

## Сборка и установка

Клонируйте репозиторий и соберите проект:

```bash
cd AppimageManager
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build -j4
cmake --install build
```

- **`CMAKE_INSTALL_PREFIX`** — куда ставить. `$HOME/.local` — только для вашего пользователя; `/usr/local` — для всех (тогда установку делают с правами администратора).
- После установки в `PATH` должны быть каталоги с бинарниками (например `~/.local/bin`). Если его нет в `PATH`, добавьте в `~/.profile` или `~/.bashrc`: `export PATH="$HOME/.local/bin:$PATH"`.

---

## Первый запуск

Демон управляется через systemd в режиме пользователя. После первой установки выполните один раз:

```bash
systemctl --user daemon-reload
systemctl --user enable appimage-manager
systemctl --user start appimage-manager
```

- **daemon-reload** — перечитать unit-файлы (нужно после установки/обновления).
- **enable** — включить автозапуск демона при входе в сессию.
- **start** — запустить демон сейчас.

Проверить статус:

```bash
systemctl --user status appimage-manager
```

Конфиг и данные создаются автоматически при первом запуске демона в каталоге:

```
~/.config/appimage-manager/
```

Там хранятся: список отслеживаемых папок, реестр AppImage, настройки запуска для каждого приложения (аргументы, переменные окружения, sandbox).

---

## Запуск GUI

После запуска демона откройте приложение:

```bash
appimage-manager-gui
```

В интерфейсе:

- **Daemon** — статус демона (работает / не работает), кнопки «Start», «Stop», «Refresh list».
- **Таблица** — список найденных AppImage (имя, путь, тип установки).
- **App settings…** — для выбранного приложения: аргументы командной строки, переменные окружения, sandbox (bwrap / firejail / без).
- **Watch directories…** — добавление и удаление папок, за которыми следит демон.
- **Install AppImage…** — установка по GitHub (username/repository) или по прямой ссылке; файл сохраняется в выбранную отслеживаемую папку, демон сам добавляет его в реестр и создаёт ярлык.

Если демон не запущен, в GUI будет отображаться «Daemon: not running» и кнопка «Start» (вызов `systemctl --user start appimage-manager`).

---

## Настройка папок (watch directories)

Демон сканирует только те каталоги, которые добавлены в список. **Каждый каталог должен уже существовать при запуске демона** — несуществующие пути пропускаются (не сканируются и не отслеживаются). Если вы правили конфиг вручную, создайте каталог до старта демона и перезапустите его.

Вложенные папки не обходятся — сканируется только первый уровень; каждую подпапку нужно добавить отдельно. Файл считается AppImage только если имя заканчивается на `.AppImage`; `.part` и `.crdownload` игнорируются.

При запуске демон выводит в stderr, какие каталоги прочитаны и какие пропущены (exists / is_directory). Запуск из терминала: `appimage-manager-daemon`; через systemd: `journalctl --user -u appimage-manager -f`.

1. В GUI нажмите **Watch directories…**.
2. **Add…** — выберите каталог (например `~/Applications` или `~/Apps`).
3. **Remove** — удалить выбранную папку из списка.
4. Сохраните (OK). Демон пересканирует папки.

---

## Полезные команды systemctl --user

| Действие        | Команда |
|-----------------|--------|
| Запустить демон | `systemctl --user start appimage-manager` |
| Остановить      | `systemctl --user stop appimage-manager` |
| Статус          | `systemctl --user status appimage-manager` |
| Включить автозапуск | `systemctl --user enable appimage-manager` |
| Отключить автозапуск | `systemctl --user disable appimage-manager` |
| Перезапустить   | `systemctl --user restart appimage-manager` |

После обновления бинарников или unit-файла снова выполните `systemctl --user daemon-reload`, при необходимости `restart`.
