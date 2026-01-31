#include "main_window.hpp"
#include "app_settings_dialog.hpp"
#include "watch_directories_dialog.hpp"
#include "install_app_image_dialog.hpp"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QProcess>
#include <QDBusArgument>
#include <QDBusReply>
#include <QDBusConnection>
#include <QResizeEvent>

namespace appimage_manager::gui {

namespace {

constexpr const char* dbus_service = "org.appimage.Manager1";
constexpr const char* dbus_path = "/org/appimage/Manager1";
constexpr const char* systemd_unit = "appimage-manager";

}

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , dbus_(new QDBusInterface(dbus_service, dbus_path, dbus_service,
                             QDBusConnection::sessionBus(), this))
  , status_timer_(new QTimer(this)) {
  setup_ui();
  connect(status_timer_, &QTimer::timeout, this, &MainWindow::refresh_daemon_status);
  status_timer_->start(2000);
  refresh_daemon_status();
  QTimer::singleShot(800, this, &MainWindow::refresh_daemon_status);
}

void MainWindow::setup_ui() {
  auto* central = new QWidget(this);
  auto* layout = new QVBoxLayout(central);

  auto* daemon_box = new QGroupBox(tr("Daemon"), this);
  auto* daemon_layout = new QHBoxLayout(daemon_box);
  status_label_ = new QLabel(tr("Daemon: checking..."), this);
  start_btn_ = new QPushButton(tr("Start"), this);
  stop_btn_ = new QPushButton(tr("Stop"), this);
  restart_btn_ = new QPushButton(tr("Restart"), this);
  autostart_btn_ = new QPushButton(tr("Autostart"), this);
  refresh_btn_ = new QPushButton(tr("Refresh list"), this);
  daemon_layout->addWidget(status_label_);
  daemon_layout->addWidget(start_btn_);
  daemon_layout->addWidget(stop_btn_);
  daemon_layout->addWidget(restart_btn_);
  daemon_layout->addWidget(autostart_btn_);
  daemon_layout->addWidget(refresh_btn_);
  app_settings_btn_ = new QPushButton(tr("App settings..."), this);
  watch_dirs_btn_ = new QPushButton(tr("Watch directories..."), this);
  install_btn_ = new QPushButton(tr("Install AppImage..."), this);
  remove_btn_ = new QPushButton(tr("Remove..."), this);
  daemon_layout->addWidget(app_settings_btn_);
  daemon_layout->addWidget(watch_dirs_btn_);
  daemon_layout->addWidget(install_btn_);
  daemon_layout->addWidget(remove_btn_);
  daemon_layout->addStretch();
  layout->addWidget(daemon_box);

  table_ = new QTableWidget(0, 3, this);
  table_->setHorizontalHeaderLabels({ tr("Name"), tr("Path"), tr("Install type") });
  table_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
  table_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
  table_->setSortingEnabled(true);
  layout->addWidget(table_);
  update_table_last_column_width();

  setCentralWidget(central);
  setWindowTitle(tr("AppImage Manager"));
  resize(700, 400);

  connect(start_btn_, &QPushButton::clicked, this, &MainWindow::start_daemon);
  connect(stop_btn_, &QPushButton::clicked, this, &MainWindow::stop_daemon);
  connect(restart_btn_, &QPushButton::clicked, this, &MainWindow::restart_daemon);
  connect(autostart_btn_, &QPushButton::clicked, this, &MainWindow::toggle_autostart);
  connect(refresh_btn_, &QPushButton::clicked, this, &MainWindow::refresh_list);
  connect(app_settings_btn_, &QPushButton::clicked, this, &MainWindow::open_app_settings);
  connect(watch_dirs_btn_, &QPushButton::clicked, this, &MainWindow::open_watch_directories);
  connect(install_btn_, &QPushButton::clicked, this, &MainWindow::open_install_dialog);
  connect(remove_btn_, &QPushButton::clicked, this, &MainWindow::remove_app);
}

bool MainWindow::is_daemon_running() const {
  if (!dbus_->isValid())
    return false;
  QDBusReply<QString> reply = dbus_->call(QStringLiteral("GetStatus"));
  return reply.isValid() && reply.value() == QLatin1String("running");
}

void MainWindow::refresh_daemon_status() {
  bool running = is_daemon_running();
  update_daemon_buttons();
  if (!running) {
    table_->setRowCount(0);
    status_label_->setText(tr("Daemon: not running"));
    return;
  }
  if (!refresh_list())
    status_label_->setText(tr("Daemon: running (list unavailable)"));
  else
    status_label_->setText(tr("Daemon: running (%1 app(s))").arg(table_->rowCount()));
}

bool MainWindow::is_autostart_enabled() const {
  QProcess proc;
  proc.setProgram(QStringLiteral("systemctl"));
  proc.setArguments({ QStringLiteral("--user"), QStringLiteral("is-enabled"), QString::fromLatin1(systemd_unit) });
  proc.start();
  if (!proc.waitForFinished(3000))
    return false;
  return proc.exitCode() == 0;
}

void MainWindow::update_daemon_buttons() {
  bool running = is_daemon_running();
  start_btn_->setEnabled(!running);
  stop_btn_->setEnabled(running);
  restart_btn_->setEnabled(running);
  refresh_btn_->setEnabled(running);
  autostart_btn_->setText(is_autostart_enabled() ? tr("Disable autostart") : tr("Enable autostart"));
}

void MainWindow::update_table_last_column_width() {
  int w = table_->viewport()->width();
  table_->setColumnWidth(2, qMax(80, static_cast<int>(w * 0.2)));
}

void MainWindow::resizeEvent(QResizeEvent* event) {
  QMainWindow::resizeEvent(event);
  update_table_last_column_width();
}

bool MainWindow::refresh_list() {
  if (!dbus_->isValid()) return false;
  QDBusReply<QVariantList> reply = dbus_->call(QStringLiteral("GetAllRecords"));
  if (!reply.isValid()) return false;
  QString saved_id = selected_app_id();
  const QVariantList list = reply.value();
  table_->setSortingEnabled(false);
  table_->setRowCount(0);
  for (const QVariant& v : list) {
    QVariantMap m = qdbus_cast<QVariantMap>(v);
    QString id = m.value(QStringLiteral("id")).toString();
    int row = table_->rowCount();
    table_->insertRow(row);
    auto* name_item = new QTableWidgetItem(m.value(QStringLiteral("name")).toString());
    name_item->setData(Qt::UserRole, id);
    table_->setItem(row, 0, name_item);
    table_->setItem(row, 1, new QTableWidgetItem(m.value(QStringLiteral("path")).toString()));
    table_->setItem(row, 2, new QTableWidgetItem(m.value(QStringLiteral("install_type")).toString()));
  }
  table_->setSortingEnabled(true);
  table_->sortByColumn(0, Qt::AscendingOrder);
  if (!saved_id.isEmpty()) {
    for (int row = 0; row < table_->rowCount(); ++row) {
      QTableWidgetItem* item = table_->item(row, 0);
      if (item && item->data(Qt::UserRole).toString() == saved_id) {
        table_->setCurrentCell(row, 0);
        table_->selectRow(row);
        break;
      }
    }
  }
  return true;
}

void MainWindow::start_daemon() {
  QProcess proc;
  proc.setProgram(QStringLiteral("systemctl"));
  proc.setArguments({ QStringLiteral("--user"), QStringLiteral("start"), QString::fromLatin1(systemd_unit) });
  proc.start();
  if (!proc.waitForFinished(5000)) {
    QMessageBox::warning(this, tr("Error"), tr("Failed to start daemon."));
    return;
  }
  if (proc.exitStatus() != QProcess::NormalExit || proc.exitCode() != 0) {
    QMessageBox::warning(this, tr("Error"), tr("Daemon failed to start: %1").arg(QString::fromUtf8(proc.readAllStandardError())));
    return;
  }
  refresh_daemon_status();
  QTimer::singleShot(800, this, &MainWindow::refresh_daemon_status);
}

void MainWindow::stop_daemon() {
  QProcess proc;
  proc.setProgram(QStringLiteral("systemctl"));
  proc.setArguments({ QStringLiteral("--user"), QStringLiteral("stop"), QString::fromLatin1(systemd_unit) });
  proc.start();
  proc.waitForFinished(5000);
  refresh_daemon_status();
}

void MainWindow::restart_daemon() {
  QProcess proc;
  proc.setProgram(QStringLiteral("systemctl"));
  proc.setArguments({ QStringLiteral("--user"), QStringLiteral("restart"), QString::fromLatin1(systemd_unit) });
  proc.start();
  if (!proc.waitForFinished(5000)) {
    QMessageBox::warning(this, tr("Error"), tr("Failed to restart daemon."));
    return;
  }
  refresh_daemon_status();
  QTimer::singleShot(800, this, &MainWindow::refresh_daemon_status);
}

void MainWindow::toggle_autostart() {
  bool enable = !is_autostart_enabled();
  QProcess proc;
  proc.setProgram(QStringLiteral("systemctl"));
  proc.setArguments({ QStringLiteral("--user"), enable ? QStringLiteral("enable") : QStringLiteral("disable"), QString::fromLatin1(systemd_unit) });
  proc.start();
  proc.waitForFinished(3000);
  update_daemon_buttons();
}

QString MainWindow::selected_app_id() const {
  QList<QTableWidgetItem*> sel = table_->selectedItems();
  if (sel.isEmpty()) return QString();
  int row = table_->row(sel.first());
  QTableWidgetItem* name_item = table_->item(row, 0);
  return name_item ? name_item->data(Qt::UserRole).toString() : QString();
}

void MainWindow::open_app_settings() {
  QString id = selected_app_id();
  if (id.isEmpty()) {
    QMessageBox::information(this, tr("App settings"), tr("Select an application first."));
    return;
  }
  int row = table_->row(table_->selectedItems().first());
  QString name = table_->item(row, 0) ? table_->item(row, 0)->text() : id;
  AppSettingsDialog dlg(dbus_, id, name, this);
  dlg.load_settings();
  dlg.exec();
  if (is_daemon_running())
    refresh_list();
}

void MainWindow::open_watch_directories() {
  WatchDirectoriesDialog dlg(dbus_, this);
  dlg.exec();
  if (is_daemon_running())
    refresh_list();
}

void MainWindow::open_install_dialog() {
  InstallAppImageDialog dlg(dbus_, this);
  dlg.exec();
  if (is_daemon_running())
    refresh_list();
}

void MainWindow::remove_app() {
  QString id = selected_app_id();
  if (id.isEmpty()) {
    QMessageBox::information(this, tr("Remove"), tr("Select an application first."));
    return;
  }
  int row = table_->row(table_->selectedItems().first());
  QString name = table_->item(row, 0) ? table_->item(row, 0)->text() : id;
  
  QMessageBox::StandardButton reply = QMessageBox::question(this, tr("Remove AppImage"),
    tr("Remove \"%1\"?\n\nThis will delete the AppImage file, desktop entry, and all settings.").arg(name),
    QMessageBox::Yes | QMessageBox::No);
  
  if (reply != QMessageBox::Yes)
    return;
  
  if (!dbus_ || !dbus_->isValid()) {
    QMessageBox::warning(this, tr("Error"), tr("Daemon not available."));
    return;
  }
  
  QDBusReply<bool> dbus_reply = dbus_->call(QStringLiteral("RemoveAppImage"), id);
  if (!dbus_reply.isValid() || !dbus_reply.value()) {
    QMessageBox::warning(this, tr("Error"), tr("Failed to remove AppImage."));
    return;
  }
  
  QMessageBox::information(this, tr("Done"), tr("AppImage removed successfully."));
  refresh_list();
}

}
