#pragma once

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QDBusInterface>

namespace appimage_manager::gui {

class WatchDirectoriesDialog : public QDialog {
  Q_OBJECT
public:
  explicit WatchDirectoriesDialog(QDBusInterface* dbus, QWidget* parent = nullptr);
  void load_directories();

private Q_SLOTS:
  void add_directory();
  void remove_directory();
  void accept() override;

private:
  QDBusInterface* dbus_{nullptr};
  QListWidget* list_{nullptr};
};

}
