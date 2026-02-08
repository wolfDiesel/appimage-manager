#pragma once

#include <QMainWindow>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QAction>
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
  void remove_app();
  void run_app();
  void on_table_item_changed(QTableWidgetItem* item);
  void show_table_context_menu(const QPoint& pos);

private:
  bool eventFilter(QObject* obj, QEvent* e) override;
  void setup_ui();
  void start_rename_at_current_row();
  bool is_daemon_running() const;
  bool is_autostart_enabled() const;
  void update_daemon_buttons();
  void update_table_last_column_width();
  QString selected_app_id() const;

  QTableWidget* table_{nullptr};
  QLabel* status_label_{nullptr};
  QPushButton* daemon_btn_{nullptr};
  QAction* start_act_{nullptr};
  QAction* stop_act_{nullptr};
  QAction* restart_act_{nullptr};
  QAction* autostart_act_{nullptr};
  QAction* refresh_act_{nullptr};
  QPushButton* install_btn_{nullptr};
  QPushButton* run_btn_{nullptr};
  QDBusInterface* dbus_{nullptr};
  QTimer* status_timer_{nullptr};
  bool suppress_name_edit_{false};
  qint64 refresh_skip_until_{0};
};

}
