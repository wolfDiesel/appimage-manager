#include "watch_directories_dialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QDBusReply>
#include <QMessageBox>

namespace appimage_manager::gui {

WatchDirectoriesDialog::WatchDirectoriesDialog(QDBusInterface* dbus, QWidget* parent)
  : QDialog(parent)
  , dbus_(dbus) {
  setWindowTitle(tr("Watch directories"));
  auto* layout = new QVBoxLayout(this);
  list_ = new QListWidget(this);
  layout->addWidget(list_);
  auto* btn_layout = new QHBoxLayout();
  auto* add_btn = new QPushButton(tr("Add..."), this);
  auto* remove_btn = new QPushButton(tr("Remove"), this);
  btn_layout->addWidget(add_btn);
  btn_layout->addWidget(remove_btn);
  btn_layout->addStretch();
  layout->addLayout(btn_layout);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(buttons);
  connect(add_btn, &QPushButton::clicked, this, &WatchDirectoriesDialog::add_directory);
  connect(remove_btn, &QPushButton::clicked, this, &WatchDirectoriesDialog::remove_directory);
  load_directories();
}

void WatchDirectoriesDialog::load_directories() {
  list_->clear();
  if (!dbus_ || !dbus_->isValid()) return;
  QDBusReply<QStringList> reply = dbus_->call(QStringLiteral("GetWatchDirectories"));
  if (!reply.isValid()) return;
  for (const QString& d : reply.value())
    list_->addItem(d);
}

void WatchDirectoriesDialog::add_directory() {
  QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory to watch"));
  if (!dir.isEmpty())
    list_->addItem(dir);
}

void WatchDirectoriesDialog::remove_directory() {
  QListWidgetItem* item = list_->currentItem();
  if (item)
    delete list_->takeItem(list_->row(item));
}

void WatchDirectoriesDialog::accept() {
  if (!dbus_ || !dbus_->isValid()) {
    QDialog::accept();
    return;
  }
  QStringList dirs;
  for (int i = 0; i < list_->count(); ++i)
    dirs.append(list_->item(i)->text());
  QDBusReply<void> reply = dbus_->call(QStringLiteral("SetWatchDirectories"), dirs);
  if (!reply.isValid()) {
    QMessageBox::warning(this, tr("Error"), tr("Failed to save: %1").arg(dbus_->lastError().message()));
    return;
  }
  dbus_->call(QStringLiteral("TriggerRescan"));
  QDialog::accept();
}

}
