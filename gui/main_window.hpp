#pragma once

#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QDBusInterface>
#include <QTimer>
#include <QResizeEvent>

namespace appimage_manager::gui {

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget* parent = nullptr);

protected:
  void resizeEvent(QResizeEvent* event) override;

private Q_SLOTS:
  void refresh_daemon_status();
  bool refresh_list();
  void start_daemon();
  void stop_daemon();
  void restart_daemon();
  void toggle_autostart();
  void open_app_settings();
  void open_watch_directories();
  void open_install_dialog();

private:
  void setup_ui();
  bool is_daemon_running() const;
  bool is_autostart_enabled() const;
  void update_daemon_buttons();
  void update_table_last_column_width();
  QString selected_app_id() const;

  QTableWidget* table_{nullptr};
  QLabel* status_label_{nullptr};
  QPushButton* start_btn_{nullptr};
  QPushButton* stop_btn_{nullptr};
  QPushButton* restart_btn_{nullptr};
  QPushButton* autostart_btn_{nullptr};
  QPushButton* refresh_btn_{nullptr};
  QPushButton* app_settings_btn_{nullptr};
  QPushButton* watch_dirs_btn_{nullptr};
  QPushButton* install_btn_{nullptr};
  QDBusInterface* dbus_{nullptr};
  QTimer* status_timer_{nullptr};
};

}
