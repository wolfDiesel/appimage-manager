#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="${1:-$PROJECT_ROOT/build}"
OUTPUT="${2:-$PROJECT_ROOT/appimage-manager.AppImage}"
APPDIR="$SCRIPT_DIR/AppDir"

mkdir -p "$APPDIR/usr/bin"
cp "$SCRIPT_DIR/AppRun" "$APPDIR/AppRun"
chmod +x "$APPDIR/AppRun"
cp "$SCRIPT_DIR/appimage-manager.desktop" "$APPDIR/"
cp "$BUILD_DIR/daemon/appimage-manager-daemon" "$APPDIR/usr/bin/"
cp "$BUILD_DIR/gui/appimage-manager-gui" "$APPDIR/usr/bin/"

if [ -f "$SCRIPT_DIR/appimage-manager.png" ]; then
  cp "$SCRIPT_DIR/appimage-manager.png" "$APPDIR/"
  cp "$SCRIPT_DIR/appimage-manager.png" "$APPDIR/.DirIcon"
elif command -v convert &>/dev/null; then
  convert -size 256x256 xc:#4a90d9 "$APPDIR/appimage-manager.png"
  cp "$APPDIR/appimage-manager.png" "$APPDIR/.DirIcon"
else
  echo 'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAYAAAAfFcSJAAAADUlEQVR42mP8z8BQDwAEhQGAhKmMIQAAAABJRU5ErkJggg==' | base64 -d > "$APPDIR/appimage-manager.png"
  cp "$APPDIR/appimage-manager.png" "$APPDIR/.DirIcon"
fi

if command -v appimagetool &>/dev/null; then
  ARCH=x86_64 appimagetool "$APPDIR" "$OUTPUT"
elif [ -n "$(find /tmp /opt /usr -maxdepth 3 -name 'appimagetool*.AppImage' 2>/dev/null | head -1)" ]; then
  TOOL="$(find /tmp /opt /usr -maxdepth 3 -name 'appimagetool*.AppImage' 2>/dev/null | head -1)"
  ARCH=x86_64 "$TOOL" "$APPDIR" "$OUTPUT"
else
  echo "appimagetool not found. Install it or run:"
  echo "  wget https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage"
  echo "  chmod +x appimagetool-x86_64.AppImage"
  echo "  ARCH=x86_64 ./appimagetool-x86_64.AppImage $APPDIR $OUTPUT"
  exit 1
fi

echo "Created: $OUTPUT"
