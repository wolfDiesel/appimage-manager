#include "github_release_selector.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QJsonObject>

namespace appimage_manager::gui {

GitHubReleaseSelector::GitHubReleaseSelector(const QJsonArray& releases, QWidget* parent)
  : QDialog(parent)
  , releases_(releases) {
  setWindowTitle(tr("Select GitHub Release"));
  setMinimumSize(500, 400);
  
  auto* layout = new QVBoxLayout(this);
  layout->addWidget(new QLabel(tr("Select a release (double-click):"), this));
  
  list_ = new QListWidget(this);
  layout->addWidget(list_);
  
  for (const QJsonValue& v : releases) {
    QJsonObject rel = v.toObject();
    QString tag = rel.value(QStringLiteral("tag_name")).toString();
    QString name = rel.value(QStringLiteral("name")).toString();
    bool prerelease = rel.value(QStringLiteral("prerelease")).toBool();
    
    QString display = tag;
    if (!name.isEmpty() && name != tag)
      display += QStringLiteral(" - ") + name;
    if (prerelease)
      display += QStringLiteral(" [pre-release]");
    
    auto* item = new QListWidgetItem(display, list_);
    item->setData(Qt::UserRole, v);
  }
  
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
  layout->addWidget(buttons);
  
  connect(list_, &QListWidget::itemDoubleClicked, this, &GitHubReleaseSelector::on_item_double_clicked);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void GitHubReleaseSelector::on_item_double_clicked(QListWidgetItem* item) {
  if (!item) return;
  QJsonValue v = item->data(Qt::UserRole).toJsonValue();
  if (!v.isObject()) return;
  QJsonObject rel = v.toObject();
  selected_tag_ = rel.value(QStringLiteral("tag_name")).toString();
  selected_assets_ = rel.value(QStringLiteral("assets")).toArray();
  accept();
}

}
