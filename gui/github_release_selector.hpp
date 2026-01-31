#pragma once

#include <QDialog>
#include <QListWidget>
#include <QJsonArray>

namespace appimage_manager::gui {

class GitHubReleaseSelector : public QDialog {
  Q_OBJECT
public:
  explicit GitHubReleaseSelector(const QJsonArray& releases, QWidget* parent = nullptr);
  
  QString selected_tag() const { return selected_tag_; }
  QJsonArray selected_assets() const { return selected_assets_; }

private Q_SLOTS:
  void on_item_double_clicked(QListWidgetItem* item);

private:
  QListWidget* list_{nullptr};
  QJsonArray releases_;
  QString selected_tag_;
  QJsonArray selected_assets_;
};

}
