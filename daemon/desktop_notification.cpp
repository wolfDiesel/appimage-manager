#include "desktop_notification.hpp"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QVariantList>

namespace appimage_manager::daemon {

void notify_appimage_processed(const std::string& filename,
                               const std::string& dir_path) {
  QDBusInterface iface(QStringLiteral("org.freedesktop.Notifications"),
                       QStringLiteral("/org/freedesktop/Notifications"),
                       QStringLiteral("org.freedesktop.Notifications"),
                       QDBusConnection::sessionBus());
  if (!iface.isValid())
    return;
  QVariantList args;
  args << QStringLiteral("AppImage Manager");
  args << 0u;
  args << QString();
  args << QStringLiteral("AppImage найден");
  args << QString::fromStdString("Файл " + filename + " обработан в директории " + dir_path);
  args << QStringList();
  args << QVariantMap();
  args << 10000;
  iface.callWithArgumentList(QDBus::Block, QStringLiteral("Notify"), args);
}

}
