#pragma once

#include <QDialog>
#include <QListWidget>
#include <QJsonArray>

namespace appimage_manager::gui {

class AppImageAssetSelector : public QDialog {
  Q_OBJECT
public:
  explicit AppImageAssetSelector(const QJsonArray& assets, QWidget* parent = nullptr);
  
  QString selected_url() const { return selected_url_; }
  QString selected_name() const { return selected_name_; }

private Q_SLOTS:
  void on_item_double_clicked(QListWidgetItem* item);

private:
  QListWidget* list_{nullptr};
  QString selected_url_;
  QString selected_name_;
};

}
