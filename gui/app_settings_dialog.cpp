#include "app_settings_dialog.hpp"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSizePolicy>
#include <QDBusReply>
#include <QVariantMap>

namespace appimage_manager::gui {

AppSettingsDialog::AppSettingsDialog(QDBusInterface* dbus, const QString& app_id,
                                     const QString& app_name, QWidget* parent)
  : QDialog(parent)
  , dbus_(dbus)
  , app_id_(app_id) {
  setWindowTitle(tr("Launch settings: %1").arg(app_name));
  setMinimumWidth(480);
  setMinimumHeight(280);
  resize(520, 320);
  auto* layout = new QFormLayout(this);
  layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
  args_edit_ = new QLineEdit(this);
  args_edit_->setPlaceholderText(tr("Command line arguments"));
  args_edit_->setMinimumWidth(300);
  layout->addRow(tr("Arguments:"), args_edit_);
  env_edit_ = new QPlainTextEdit(this);
  env_edit_->setPlaceholderText(tr("KEY=VALUE per line"));
  env_edit_->setMinimumHeight(100);
  env_edit_->setSizePolicy(env_edit_->sizePolicy().horizontalPolicy(), QSizePolicy::Expanding);
  layout->addRow(tr("Environment:"), env_edit_);
  sandbox_combo_ = new QComboBox(this);
  sandbox_combo_->addItem(tr("None"), QStringLiteral("none"));
  sandbox_combo_->addItem(QStringLiteral("bwrap"), QStringLiteral("bwrap"));
  sandbox_combo_->addItem(QStringLiteral("firejail"), QStringLiteral("firejail"));
  layout->addRow(tr("Sandbox:"), sandbox_combo_);
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addRow(buttons);
  load_settings();
}

void AppSettingsDialog::load_settings() {
  if (!dbus_ || !dbus_->isValid()) return;
  QDBusReply<QVariantMap> reply = dbus_->call(QStringLiteral("GetLaunchSettings"), app_id_);
  if (!reply.isValid()) return;
  QVariantMap m = reply.value();
  args_edit_->setText(m.value(QStringLiteral("args")).toString());
  QStringList env = m.value(QStringLiteral("env")).toStringList();
  env_edit_->setPlainText(env.join(QLatin1Char('\n')));
  QString sandbox = m.value(QStringLiteral("sandbox")).toString();
  int idx = sandbox_combo_->findData(sandbox);
  if (idx >= 0)
    sandbox_combo_->setCurrentIndex(idx);
}

void AppSettingsDialog::accept() {
  if (!dbus_ || !dbus_->isValid()) {
    QDialog::accept();
    return;
  }
  QString args = args_edit_->text();
  QStringList env = env_edit_->toPlainText().split(QLatin1Char('\n'), Qt::SkipEmptyParts);
  for (QString& e : env)
    e = e.trimmed();
  QString sandbox = sandbox_combo_->currentData().toString();
  dbus_->call(QStringLiteral("SetLaunchSettings"), app_id_, args, env, sandbox);
  QDialog::accept();
}

}
