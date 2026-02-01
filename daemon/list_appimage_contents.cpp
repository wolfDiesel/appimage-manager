#include <application/extract_icon.hpp>
#include <QCoreApplication>
#include <QProcess>
#include <QStandardPaths>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  QCoreApplication app(argc, argv);
  if (argc < 2) {
    std::cerr << "Usage: list-appimage-contents <path-to-AppImage>\n";
    return 1;
  }
  std::string path = argv[1];
  auto offset_opt = appimage_manager::application::get_appimage_squashfs_offset(path);
  if (!offset_opt.has_value()) {
    std::cerr << "Could not find SquashFS offset in " << path << "\n";
    return 1;
  }
  QString unsquashfs = QStandardPaths::findExecutable(QStringLiteral("unsquashfs"));
  if (unsquashfs.isEmpty())
    unsquashfs = QStringLiteral("/usr/bin/unsquashfs");
  QProcess proc;
  proc.setProgram(unsquashfs);
  proc.setArguments({
    QStringLiteral("-o"), QString::number(offset_opt.value()),
    QStringLiteral("-l"), QString::fromStdString(path)
  });
  proc.start();
  if (!proc.waitForFinished(15000)) {
    std::cerr << "unsquashfs -l failed or timed out\n";
    return 1;
  }
  if (proc.exitCode() != 0) {
    std::cerr << "unsquashfs exited with " << proc.exitCode() << "\n";
    return 1;
  }
  QByteArray out = proc.readAllStandardOutput();
  for (const QByteArray& line : out.split('\n')) {
    std::string s = line.constData();
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n'))
      s.pop_back();
    if (s.size() >= 4u) {
      bool match = (s.size() >= 8u && s.compare(s.size() - 8, 8, ".desktop") == 0) ||
                   (s.size() >= 4u && s.compare(s.size() - 4, 4, ".png") == 0) ||
                   (s.size() >= 4u && s.compare(s.size() - 4, 4, ".svg") == 0);
      if (match)
        std::cout << s << "\n";
    }
  }
  return 0;
}
