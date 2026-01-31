#include "install_app_image_dialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QDBusReply>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QLabel>

namespace appimage_manager::gui {

namespace {

const char github_user_agent[] = "AppImage-Manager-GUI";

}

InstallAppImageDialog::InstallAppImageDialog(QDBusInterface* dbus, QWidget* parent)
  : QDialog(parent)
  , dbus_(dbus)
  , nam_(new QNetworkAccessManager(this)) {
  setWindowTitle(tr("Install AppImage"));
  auto* layout = new QVBoxLayout(this);

  auto* source_group = new QGroupBox(tr("Source"), this);
  auto* source_layout = new QVBoxLayout(source_group);
  github_radio_ = new QRadioButton(tr("GitHub (username/repository)"), this);
  github_edit_ = new QLineEdit(this);
  github_edit_->setPlaceholderText(QStringLiteral("owner/repo"));
  url_radio_ = new QRadioButton(tr("Direct URL"), this);
  url_edit_ = new QLineEdit(this);
  url_edit_->setPlaceholderText(QStringLiteral("https://..."));
  github_radio_->setChecked(true);
  source_layout->addWidget(github_radio_);
  source_layout->addWidget(github_edit_);
  source_layout->addWidget(url_radio_);
  source_layout->addWidget(url_edit_);
  layout->addWidget(source_group);

  auto* target_layout = new QHBoxLayout();
  target_layout->addWidget(new QLabel(tr("Save to directory:"), this));
  target_dir_combo_ = new QComboBox(this);
  target_dir_combo_->setMinimumWidth(300);
  target_layout->addWidget(target_dir_combo_, 1);
  layout->addLayout(target_layout);

  progress_ = new QProgressBar(this);
  progress_->setVisible(false);
  layout->addWidget(progress_);

  auto* buttons = new QDialogButtonBox(this);
  install_btn_ = new QPushButton(tr("Install"), this);
  cancel_btn_ = buttons->addButton(QDialogButtonBox::Close);
  buttons->addButton(install_btn_, QDialogButtonBox::AcceptRole);
  layout->addWidget(buttons);

  connect(install_btn_, &QPushButton::clicked, this, &InstallAppImageDialog::start_install);
  connect(cancel_btn_, &QPushButton::clicked, this, [this]() {
    if (active_reply_) {
      active_reply_->abort();
    } else {
      reject();
    }
  });

  load_watch_directories();
}

void InstallAppImageDialog::load_watch_directories() {
  target_dir_combo_->clear();
  if (!dbus_ || !dbus_->isValid()) return;
  QDBusReply<QStringList> reply = dbus_->call(QStringLiteral("GetWatchDirectories"));
  if (!reply.isValid()) return;
  const QStringList dirs = reply.value();
  for (const QString& d : dirs)
    target_dir_combo_->addItem(d, d);
  if (dirs.isEmpty()) {
    install_btn_->setEnabled(false);
    install_btn_->setToolTip(tr("Add at least one watch directory first."));
  }
}

void InstallAppImageDialog::set_busy(bool busy) {
  progress_->setVisible(busy);
  install_btn_->setEnabled(!busy);
  github_edit_->setEnabled(!busy);
  url_edit_->setEnabled(!busy);
  github_radio_->setEnabled(!busy);
  url_radio_->setEnabled(!busy);
  target_dir_combo_->setEnabled(!busy);
  if (!busy) {
    active_reply_ = nullptr;
    if (output_file_ && output_file_->isOpen()) {
      output_file_->close();
      delete output_file_;
      output_file_ = nullptr;
    }
  }
}

QString InstallAppImageDialog::github_release_url(const QString& spec) const {
  QString s = spec.trimmed();
  if (s.isEmpty()) return QString();
  s.replace(QStringLiteral("https://github.com/"), QString());
  s.replace(QStringLiteral("github.com/"), QString());
  int i = s.indexOf(QLatin1Char('/'));
  if (i <= 0 || i == s.size() - 1) return QString();
  return QStringLiteral("https://api.github.com/repos/%1/releases/latest").arg(s);
}

void InstallAppImageDialog::start_install() {
  const QString target = target_dir_combo_->currentData().toString();
  if (target.isEmpty()) {
    QMessageBox::warning(this, tr("Error"), tr("Select a target directory."));
    return;
  }
  if (github_radio_->isChecked()) {
    QString api_url = github_release_url(github_edit_->text());
    if (api_url.isEmpty()) {
      QMessageBox::warning(this, tr("Error"), tr("Enter GitHub repository as username/repository."));
      return;
    }
    set_busy(true);
    progress_->setRange(0, 0);
    QNetworkRequest req{QUrl(api_url)};
    req.setRawHeader("User-Agent", github_user_agent);
    req.setRawHeader("Accept", "application/vnd.github.v3+json");
    QNetworkReply* reply = nam_->get(req);
    connect(reply, &QNetworkReply::finished, this, &InstallAppImageDialog::fetch_github_finished);
    active_reply_ = reply;
    return;
  }
  QUrl url(url_edit_->text().trimmed());
  if (!url.isValid() || url.scheme().isEmpty()) {
    QMessageBox::warning(this, tr("Error"), tr("Enter a valid URL."));
    return;
  }
  start_download(url, suggested_filename(url));
}

void InstallAppImageDialog::fetch_github_finished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) return;
  reply->deleteLater();
  active_reply_ = nullptr;
  set_busy(false);
  if (reply->error() != QNetworkReply::NoError) {
    QMessageBox::warning(this, tr("Error"), tr("GitHub request failed: %1").arg(reply->errorString()));
    return;
  }
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);
  if (err.error != QJsonParseError::NoError || !doc.isObject()) {
    QMessageBox::warning(this, tr("Error"), tr("Invalid GitHub response."));
    return;
  }
  QJsonObject obj = doc.object();
  QJsonArray assets = obj.value(QStringLiteral("assets")).toArray();
  QString download_url;
  QString asset_name;
  for (const QJsonValue& v : assets) {
    QJsonObject a = v.toObject();
    QString name = a.value(QStringLiteral("name")).toString();
    if (name.endsWith(QLatin1String(".AppImage"), Qt::CaseInsensitive)) {
      download_url = a.value(QStringLiteral("browser_download_url")).toString();
      asset_name = name;
      break;
    }
  }
  if (download_url.isEmpty()) {
    QMessageBox::warning(this, tr("Error"), tr("No AppImage asset found in latest release."));
    return;
  }
  start_download(QUrl(download_url), asset_name);
}

void InstallAppImageDialog::start_download(const QUrl& url, const QString& suggested_name) {
  const QString target_dir = target_dir_combo_->currentData().toString();
  if (target_dir.isEmpty()) return;
  QString name = suggested_name;
  if (name.isEmpty() || !name.endsWith(QLatin1String(".AppImage"), Qt::CaseInsensitive))
    name = QStringLiteral("downloaded.AppImage");
  target_path_ = target_dir + QLatin1Char('/') + name;
  QString part_path = target_path_ + QStringLiteral(".part");
  output_file_ = new QFile(part_path, this);
  if (!output_file_->open(QIODevice::WriteOnly)) {
    QMessageBox::warning(this, tr("Error"), tr("Cannot write to: %1").arg(part_path));
    delete output_file_;
    output_file_ = nullptr;
    return;
  }
  set_busy(true);
  progress_->setRange(0, 100);
  progress_->setValue(0);
  QNetworkRequest req(url);
  req.setRawHeader("User-Agent", github_user_agent);
  active_reply_ = nam_->get(req);
  connect(active_reply_, &QNetworkReply::downloadProgress, this, &InstallAppImageDialog::download_progress);
  connect(active_reply_, &QNetworkReply::readyRead, this, [this]() {
    if (output_file_ && output_file_->isOpen() && active_reply_)
      output_file_->write(active_reply_->readAll());
  });
  connect(active_reply_, &QNetworkReply::finished, this, &InstallAppImageDialog::download_finished);
}

QString InstallAppImageDialog::suggested_filename(const QUrl& url) const {
  QString name = QFileInfo(url.path()).fileName();
  if (name.isEmpty() || !name.endsWith(QLatin1String(".AppImage"), Qt::CaseInsensitive))
    name = QStringLiteral("downloaded.AppImage");
  return name;
}

void InstallAppImageDialog::download_progress(qint64 received, qint64 total) {
  if (total > 0)
    progress_->setValue(static_cast<int>((received * 100) / total));
}

void InstallAppImageDialog::download_finished() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply) return;
  if (output_file_ && output_file_->isOpen()) {
    output_file_->write(reply->readAll());
    output_file_->close();
  }
  const bool ok = (reply->error() == QNetworkReply::NoError);
  const QString part_path = target_path_ + QStringLiteral(".part");
  if (ok && output_file_) {
    QFile::remove(target_path_);
    if (!output_file_->rename(target_path_))
      QMessageBox::warning(this, tr("Error"), tr("Cannot rename to: %1").arg(target_path_));
  }
  if (output_file_) {
    delete output_file_;
    output_file_ = nullptr;
  }
  reply->deleteLater();
  active_reply_ = nullptr;
  set_busy(false);
  if (!ok) {
    QFile::remove(part_path);
    if (reply->error() != QNetworkReply::OperationCanceledError)
      QMessageBox::warning(this, tr("Error"), tr("Download failed: %1").arg(reply->errorString()));
    return;
  }
  if (dbus_ && dbus_->isValid())
    dbus_->call(QStringLiteral("TriggerRescan"));
  QMessageBox::information(this, tr("Done"), tr("Downloaded to %1. Daemon will add it to the list.").arg(target_path_));
  accept();
}

}
