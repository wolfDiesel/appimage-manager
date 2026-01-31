# AppImage Manager — руководство пользователя

Приложение состоит из **демона** (фоновый сервис) и **GUI** (окно управления). Демон следит за выбранными папками, находит AppImage-файлы и создаёт ярлыки в меню приложений. GUI управляет демоном, настройками и установкой. Всё работает в режиме пользователя (`systemctl --user`).

---

## Содержание

1. [Зависимости](#зависимости)
2. [Установка](#установка)
3. [Запуск из AppImage](#запуск-из-appimage)
4. [Сборка из исходников](#сборка-из-исходников)
5. [Первый запуск демона](#первый-запуск-демона)
6. [Интерфейс GUI](#интерфейс-gui)
7. [Установка AppImage (GitHub и URL)](#установка-appimage-github-и-url)
8. [Удаление AppImage](#удаление-appimage)
9. [Настройка папок (watch directories)](#настройка-папок-watch-directories)
10. [Настройки приложения (App settings)](#настройки-приложения-app-settings)
11. [Команды systemctl](#команды-systemctl)

---

## Зависимости

- **ОС:** Linux с systemd (user session), D-Bus, inotify  
- **Для сборки:** C++20, CMake 3.16+, Qt6 (Core, Widgets, DBus, Network), nlohmann-json  

Пример (Ubuntu 24.04 / Fedora):

```bash
# Ubuntu / Debian
sudo apt install build-essential cmake qt6-base-dev nlohmann-json3-dev

# Fedora
sudo dnf install gcc-c++ cmake qt6-qtbase-devel json-devel
```

---

## Установка

### Запуск из AppImage

1. Скачайте `appimage-manager-*.AppImage` из [Releases](https://github.com/wolfDiesel/appimage-manager/releases).
2. Сделайте файл исполняемым:  
   `chmod +x appimage-manager-*.AppImage`
3. Запустите двойным щелчком или из терминала:  
   `./appimage-manager-*.AppImage`

**При первом запуске** AppImage сам:

- создаёт конфиг в `~/.config/appimage-manager/`;
- создаёт unit systemd и пункт в меню приложений;
- запускает демон.

**При каждом следующем запуске** (в т.ч. после обновления):

- останавливается текущий демон;
- в unit и в пункте меню подставляется путь к **текущему** AppImage (тот, который вы только что запустили);
- демон запускается заново.

Таким образом, чтобы обновиться, достаточно скачать новый AppImage и запустить его — старый демон остановится, пути обновятся на новый файл, перезапускать ничего вручную не нужно.

Если AppImage Manager (тот же файл) лежит в одной из отслеживаемых папок, он **не** появится в списке приложений и его нельзя случайно удалить через «Remove».

---

### Сборка из исходников

```bash
git clone https://github.com/wolfDiesel/appimage-manager.git
cd appimage-manager
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build -j4
cmake --install build
```

- **`CMAKE_INSTALL_PREFIX`** — каталог установки (`$HOME/.local` — только для пользователя; для всех — `/usr/local`, установка с правами администратора).
- Убедитесь, что в `PATH` есть каталог с бинарниками (например `~/.local/bin`). При необходимости добавьте в `~/.profile` или `~/.bashrc`:  
  `export PATH="$HOME/.local/bin:$PATH"`

---

## Первый запуск демона

Если ставили из пакета/исходников, демон не стартует сам. Один раз выполните:

```bash
systemctl --user daemon-reload
systemctl --user enable appimage-manager   # автозапуск при входе
systemctl --user start appimage-manager    # запуск сейчас
```

Проверка статуса:

```bash
systemctl --user status appimage-manager
```

Конфиг и данные создаются при первом запуске в каталоге:

```
~/.config/appimage-manager/
```

Там хранятся: список отслеживаемых папок, реестр AppImage, настройки запуска (аргументы, переменные окружения, sandbox) для каждого приложения.

---

## Интерфейс GUI

После запуска демона откройте приложение: **AppImage Manager** из меню или `appimage-manager-gui` из терминала.

### Блок «Daemon»

- **Статус** — «Daemon: running (N app(s))» или «Daemon: not running».
- **Start** — запустить демон (`systemctl --user start appimage-manager`).
- **Stop** — остановить демон.
- **Restart** — перезапустить демон.
- **Autostart** — включить/выключить автозапуск демона при входе в сессию (в GUI отображается как «Enable autostart» / «Disable autostart»).
- **Refresh list** — обновить список приложений.
- **App settings…** — настройки запуска для выбранного в таблице приложения.
- **Watch directories…** — добавление и удаление папок, за которыми следит демон.
- **Install AppImage…** — установка AppImage с GitHub или по прямой ссылке.
- **Remove…** — удалить выбранное приложение (файл, ярлык, запись в реестре и настройки).

### Таблица приложений

Список найденных AppImage: **Name**, **Path**, **Install type**. Сортировка по имени; при обновлении списка выбранная строка сохраняется.

Если демон не запущен, в GUI отображается «Daemon: not running» и доступна кнопка «Start».

---

## Установка AppImage (GitHub и URL)

**Install AppImage…** в GUI.

### Вариант «GitHub (username/repository)»

1. Введите репозиторий, например: `wolfDiesel/appimage-manager`.
2. Нажмите **Install**.
3. Загружается список **последних 10 релизов**. Выберите нужный релиз (двойной щелчок).
4. Откроется список **ассетов** с расширением `.appimage` (регистр не важен). Выберите нужный файл (двойной щелчок).
5. Файл скачивается в выбранную **watch directory**. Демон сам добавляет его в реестр и создаёт ярлык в меню.

### Вариант «Direct URL»

1. Вставьте прямую ссылку на файл `.AppImage`.
2. Нажмите **Install**.
3. Файл сохраняется в выбранную watch directory; демон добавляет его в список и создаёт ярлык.

Сначала должна быть добавлена хотя бы одна watch directory (**Watch directories…**).

---

## Удаление AppImage

1. Выберите приложение в таблице.
2. Нажмите **Remove…**.
3. Подтвердите удаление.

При удалении:

- удаляется **файл** AppImage с диска;
- удаляется **ярлык** в меню приложений (`~/.local/share/applications/`);
- удаляются **настройки запуска** для этого приложения (аргументы, переменные окружения, sandbox);
- запись убирается из **реестра** демона.

Список в GUI обновляется автоматически.

---

## Настройка папок (watch directories)

Демон сканирует только те каталоги, которые вы добавили. **Каталог должен существовать** на момент запуска демона — несуществующие пути не сканируются и не отслеживаются.

Сканируется **только первый уровень** каталога (вложенные папки не обходятся). Файл считается AppImage, если имя заканчивается на `.AppImage`; файлы `.part` и `.crdownload` игнорируются.

**В GUI:**

1. **Watch directories…** → **Add…** — выберите каталог (например `~/Apps` или `~/Applications`).
2. **Remove** — убрать выбранную папку из списка.
3. **OK** — демон пересканирует папки.

Логи демона (какие каталоги прочитаны):  
`journalctl --user -u appimage-manager -f`

---

## Настройки приложения (App settings)

Для выбранного в таблице приложения можно задать:

- **Arguments** — аргументы командной строки при запуске.
- **Environment** — переменные окружения (по одной на строку).
- **Sandbox** — без sandbox, bwrap или firejail.

Эти настройки сохраняются и подставляются в ярлык в меню приложений. После изменения имеет смысл нажать **Refresh list** в главном окне.

---

## Команды systemctl

| Действие            | Команда |
|---------------------|--------|
| Запустить демон     | `systemctl --user start appimage-manager` |
| Остановить          | `systemctl --user stop appimage-manager` |
| Статус              | `systemctl --user status appimage-manager` |
| Включить автозапуск | `systemctl --user enable appimage-manager` |
| Отключить автозапуск| `systemctl --user disable appimage-manager` |
| Перезапустить       | `systemctl --user restart appimage-manager` |

После обновления бинарников или unit-файла выполните:  
`systemctl --user daemon-reload`  
и при необходимости `restart`.
