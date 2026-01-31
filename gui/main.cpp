#include "main_window.hpp"
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  app.setApplicationName(QStringLiteral("AppImage Manager"));
  appimage_manager::gui::MainWindow window;
  window.show();
  return app.exec();
}
