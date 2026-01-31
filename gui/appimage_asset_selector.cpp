#include "appimage_asset_selector.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QJsonObject>
#include <QMessageBox>

namespace appimage_manager::gui {

AppImageAssetSelector::AppImageAssetSelector(const QJsonArray& assets, QWidget* parent)
  : QDialog(parent) {
  setWindowTitle(tr("Select AppImage Asset"));
  setMinimumSize(500, 300);
  
  auto* layout = new QVBoxLayout(this);
  layout->addWidget(new QLabel(tr("Select an AppImage asset (double-click):"), this));
  
  list_ = new QListWidget(this);
  layout->addWidget(list_);
  
  for (const QJsonValue& v : assets) {
    QJsonObject asset = v.toObject();
    QString name = asset.value(QStringLiteral("name")).toString();
    
    if (!name.endsWith(QLatin1String(".appimage"), Qt::CaseInsensitive))
      continue;
    
    qint64 size = asset.value(QStringLiteral("size")).toInteger();
    QString size_str;
    if (size > 1024 * 1024)
      size_str = QStringLiteral(" (%1 MB)").arg(size / (1024.0 * 1024.0), 0, 'f', 1);
    else if (size > 1024)
      size_str = QStringLiteral(" (%1 KB)").arg(size / 1024.0, 0, 'f', 1);
    
    auto* item = new QListWidgetItem(name + size_str, list_);
    item->setData(Qt::UserRole, v);
  }
  
  if (list_->count() == 0) {
    QMessageBox::warning(this, tr("No Assets"), tr("No AppImage assets found in this release."));
    reject();
    return;
  }
  
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
  layout->addWidget(buttons);
  
  connect(list_, &QListWidget::itemDoubleClicked, this, &AppImageAssetSelector::on_item_double_clicked);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void AppImageAssetSelector::on_item_double_clicked(QListWidgetItem* item) {
  if (!item) return;
  QJsonValue v = item->data(Qt::UserRole).toJsonValue();
  if (!v.isObject()) return;
  QJsonObject asset = v.toObject();
  selected_url_ = asset.value(QStringLiteral("browser_download_url")).toString();
  selected_name_ = asset.value(QStringLiteral("name")).toString();
  if (!selected_url_.isEmpty() && !selected_name_.isEmpty())
    accept();
}

}
