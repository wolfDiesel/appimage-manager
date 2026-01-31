#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QString>
#include <QDBusInterface>

namespace appimage_manager::gui {

class AppSettingsDialog : public QDialog {
  Q_OBJECT
public:
  explicit AppSettingsDialog(QDBusInterface* dbus, const QString& app_id,
                             const QString& app_name, QWidget* parent = nullptr);
  void load_settings();

private Q_SLOTS:
  void accept() override;

private:
  QDBusInterface* dbus_{nullptr};
  QString app_id_;
  QLineEdit* args_edit_{nullptr};
  QPlainTextEdit* env_edit_{nullptr};
  QComboBox* sandbox_combo_{nullptr};
};

}
