#include "dbus_manager_adaptor.hpp"
#include <domain/entities/app_image_record.hpp>
#include <domain/entities/install_type.hpp>
#include <domain/entities/launch_settings.hpp>
#include <application/generate_desktop.hpp>
#include <QDBusConnection>
#include <QVariantMap>
#include <filesystem>

namespace appimage_manager::daemon {

namespace {

QString install_type_to_string(domain::InstallType t) {
  switch (t) {
    case domain::InstallType::Downloaded: return QStringLiteral("Downloaded");
    case domain::InstallType::GitHub: return QStringLiteral("GitHub");
    case domain::InstallType::Direct: return QStringLiteral("Direct");
  }
  return QStringLiteral("Downloaded");
}

QString sandbox_to_string(domain::SandboxMechanism s) {
  switch (s) {
    case domain::SandboxMechanism::Bwrap: return QStringLiteral("bwrap");
    case domain::SandboxMechanism::Firejail: return QStringLiteral("firejail");
    default: return QStringLiteral("none");
  }
}

domain::SandboxMechanism string_to_sandbox(const QString& s) {
  if (s == QLatin1String("bwrap")) return domain::SandboxMechanism::Bwrap;
  if (s == QLatin1String("firejail")) return domain::SandboxMechanism::Firejail;
  return domain::SandboxMechanism::None;
}

}

DBusManagerAdaptor::DBusManagerAdaptor(domain::RegistryRepository& registry,
                                     domain::ConfigRepository& config_repository,
                                     domain::LaunchSettingsRepository& launch_settings_repository,
                                     const std::string& applications_dir,
                                     DirectoryWatcher* watcher,
                                     QObject* parent)
  : QDBusAbstractAdaptor(parent)
  , registry_(&registry)
  , config_repository_(&config_repository)
  , launch_settings_repository_(&launch_settings_repository)
  , applications_dir_(applications_dir)
  , watcher_(watcher) {}

QVariantList DBusManagerAdaptor::GetAllRecords() const {
  QVariantList list;
  for (const auto& r : registry_->all()) {
    QVariantMap m;
    m.insert(QStringLiteral("id"), QString::fromStdString(r.id));
    m.insert(QStringLiteral("path"), QString::fromStdString(r.path));
    m.insert(QStringLiteral("name"), QString::fromStdString(r.name));
    m.insert(QStringLiteral("install_type"), install_type_to_string(r.install_type));
    m.insert(QStringLiteral("added_at"), QString::fromStdString(r.added_at));
    list.append(m);
  }
  return list;
}

QStringList DBusManagerAdaptor::GetWatchDirectories() const {
  auto config = config_repository_->load();
  QStringList list;
  for (const auto& d : config.watch_directories)
    list.append(QString::fromStdString(d));
  return list;
}

void DBusManagerAdaptor::SetWatchDirectories(const QStringList& directories) {
  domain::Config config;
  for (const QString& d : directories)
    config.watch_directories.push_back(d.toStdString());
  config_repository_->save(config);
  if (watcher_)
    watcher_->set_config(config);
}

void DBusManagerAdaptor::TriggerRescan() {
  if (watcher_)
    watcher_->trigger_rescan();
}

QString DBusManagerAdaptor::GetStatus() const {
  return QStringLiteral("running");
}

QVariantMap DBusManagerAdaptor::GetLaunchSettings(const QString& app_id) const {
  QVariantMap m;
  auto settings = launch_settings_repository_->load(app_id.toStdString());
  if (!settings) {
    m.insert(QStringLiteral("args"), QString());
    m.insert(QStringLiteral("env"), QStringList());
    m.insert(QStringLiteral("sandbox"), QStringLiteral("none"));
    return m;
  }
  m.insert(QStringLiteral("args"), QString::fromStdString(settings->args));
  QStringList env;
  for (const auto& e : settings->env)
    env.append(QString::fromStdString(e));
  m.insert(QStringLiteral("env"), env);
  m.insert(QStringLiteral("sandbox"), sandbox_to_string(settings->sandbox));
  return m;
}

void DBusManagerAdaptor::SetLaunchSettings(const QString& app_id, const QString& args,
                                          const QStringList& env, const QString& sandbox) {
  domain::LaunchSettings settings;
  settings.args = args.toStdString();
  for (const auto& e : env)
    settings.env.push_back(e.toStdString());
  settings.sandbox = string_to_sandbox(sandbox);
  launch_settings_repository_->save(app_id.toStdString(), settings);
  auto record = registry_->by_id(app_id.toStdString());
  if (record) {
    std::string icons_dir = (std::filesystem::path(applications_dir_).parent_path() / "appimage-manager" / "icons").string();
    std::string icon_path = application::icon_file_path(record->id, icons_dir);
    if (!std::filesystem::is_regular_file(icon_path))
      icon_path.clear();
    application::generate_desktop(*record, settings, applications_dir_, icon_path);
  }
}

bool DBusManagerAdaptor::RemoveAppImage(const QString& app_id) {
  std::string id = app_id.toStdString();
  auto record = registry_->by_id(id);
  if (!record)
    return false;
  
  std::error_code ec;
  std::filesystem::remove(record->path, ec);
  application::remove_desktop(id, record->name, applications_dir_);
  launch_settings_repository_->remove(id);
  registry_->remove(id);
  
  if (watcher_)
    watcher_->trigger_rescan();
  
  return true;
}

}
