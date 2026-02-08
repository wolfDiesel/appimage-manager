#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QListWidget>
#include <QRadioButton>
#include <QProgressBar>
#include <QPushButton>
#include <QDBusInterface>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>

namespace appimage_manager::gui {

class InstallAppImageDialog : public QDialog {
  Q_OBJECT
public:
  explicit InstallAppImageDialog(QDBusInterface* dbus, QWidget* parent = nullptr);

private Q_SLOTS:
  void start_install();
  void download_finished();
  void download_progress(qint64 received, qint64 total);
  void fetch_github_releases_finished();
  void fetch_github_search_finished();
  void fetch_github_releases_for_search_finished();
  void on_github_search_double_clicked(QListWidgetItem* item);

private:
  void load_watch_directories();
  void set_busy(bool busy);
  QString github_releases_url(const QString& spec) const;
  void start_download(const QUrl& url, const QString& suggested_name);
  QString suggested_filename(const QUrl& url) const;

  bool eventFilter(QObject* obj, QEvent* e) override;
  void run_github_search();
  void fetch_releases_for_repo(const QString& full_name);
  void handle_releases_response(const QJsonArray& releases);

  QDBusInterface* dbus_{nullptr};
  QNetworkAccessManager* nam_{nullptr};
  QRadioButton* github_radio_{nullptr};
  QLineEdit* github_edit_{nullptr};
  QLineEdit* github_search_edit_{nullptr};
  QPushButton* github_search_btn_{nullptr};
  QListWidget* github_search_list_{nullptr};
  QRadioButton* url_radio_{nullptr};
  QLineEdit* url_edit_{nullptr};
  QComboBox* target_dir_combo_{nullptr};
  QProgressBar* progress_{nullptr};
  QPushButton* install_btn_{nullptr};
  QPushButton* cancel_btn_{nullptr};
  QNetworkReply* active_reply_{nullptr};
  QFile* output_file_{nullptr};
  QString target_path_;
};

}
