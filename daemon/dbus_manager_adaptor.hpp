#pragma once

#include <domain/entities/config.hpp>
#include <domain/repositories/registry_repository.hpp>
#include <domain/repositories/config_repository.hpp>
#include <domain/repositories/launch_settings_repository.hpp>
#include <directory_watcher.hpp>
#include <QDBusAbstractAdaptor>
#include <QVariantList>
#include <QVariantMap>
#include <QStringList>
#include <QString>

namespace appimage_manager::daemon {

class DirectoryWatcher;

class DBusManagerAdaptor : public QDBusAbstractAdaptor {
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.appimage.Manager1")
public:
  explicit DBusManagerAdaptor(domain::RegistryRepository& registry,
                              domain::ConfigRepository& config_repository,
                              domain::LaunchSettingsRepository& launch_settings_repository,
                              const std::string& applications_dir,
                              DirectoryWatcher* watcher,
                              QObject* parent);

public Q_SLOTS:
  QVariantList GetAllRecords() const;
  QStringList GetWatchDirectories() const;
  void SetWatchDirectories(const QStringList& directories);
  void TriggerRescan();
  QString GetStatus() const;
  QVariantMap GetLaunchSettings(const QString& app_id) const;
  void SetLaunchSettings(const QString& app_id, const QString& args,
                         const QStringList& env, const QString& sandbox);

private:
  domain::RegistryRepository* registry_;
  domain::ConfigRepository* config_repository_;
  domain::LaunchSettingsRepository* launch_settings_repository_;
  std::string applications_dir_;
  DirectoryWatcher* watcher_;
};

}
