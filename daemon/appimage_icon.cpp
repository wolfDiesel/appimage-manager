#include "appimage_icon.hpp"
#include "appimage_icon_paths.hpp"
#include <application/extract_icon.hpp>
#include <application/generate_desktop.hpp>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <string>

namespace fs = std::filesystem;

namespace appimage_manager::daemon {

namespace {

bool looks_like_image(const fs::path& p) {
  std::ifstream f(p, std::ios::binary);
  if (!f)
    return false;
  std::array<std::uint8_t, 8> buf{};
  if (!f.read(reinterpret_cast<char*>(buf.data()), buf.size()))
    return false;
  if (buf[0] == 0x89 && buf[1] == 0x50 && buf[2] == 0x4E && buf[3] == 0x47)
    return true;
  if (buf[0] == '<' && (buf[1] == '?' || buf[1] == 's') &&
      (buf[2] == 'x' || buf[2] == 'v') && (buf[3] == 'm' || buf[3] == 'g'))
    return true;
  if (buf[0] == '<' && buf[1] == 's' && buf[2] == 'v' && buf[3] == 'g')
    return true;
  return false;
}

QString unsquashfs_path() {
  QString p = QStandardPaths::findExecutable(QStringLiteral("unsquashfs"));
  if (!p.isEmpty())
    return p;
  return QStringLiteral("/usr/bin/unsquashfs");
}

}

std::string extract_icon_from_appimage(const std::string& appimage_path,
                                       const std::string& icons_dir,
                                       const std::string& record_id) {
  auto offset_opt = application::get_appimage_squashfs_offset(appimage_path);
  if (!offset_opt.has_value())
    return {};
  QTemporaryDir temp_dir;
  if (!temp_dir.isValid())
    return {};
  std::string temp_path = temp_dir.path().toStdString();
  std::string list_path = (fs::path(temp_path) / "extract_list").string();
  {
    std::ofstream list_file(list_path);
    if (!list_file)
      return {};
    list_file << ".DirIcon\n";
  }
  QProcess proc;
  proc.setProgram(unsquashfs_path());
  proc.setArguments({
    QStringLiteral("-o"), QString::number(offset_opt.value()),
    QStringLiteral("-follow-symlinks"),
    QStringLiteral("-d"), QString::fromStdString(temp_path),
    QStringLiteral("-ef"), QString::fromStdString(list_path),
    QString::fromStdString(appimage_path)
  });
  proc.start();
  if (!proc.waitForFinished(30000))
    return {};
  int code = proc.exitCode();
  if (code == 1)
    return {};
  fs::path extracted = fs::path(temp_path) / ".DirIcon";
  fs::path icon_file = extracted;
  if (fs::exists(extracted)) {
    if (fs::is_symlink(extracted)) {
      fs::path target = fs::read_symlink(extracted);
      icon_file = extracted.parent_path() / target;
    }
    if (fs::is_regular_file(icon_file) && !looks_like_image(icon_file))
      icon_file = fs::path();
  }
  auto run_unsquashfs_ef = [&](const std::string& entry) {
    std::ofstream out(list_path);
    if (!out)
      return false;
    out << entry << "\n";
    out.close();
    QProcess p;
    p.setProgram(unsquashfs_path());
    p.setArguments({
      QStringLiteral("-o"), QString::number(offset_opt.value()),
      QStringLiteral("-follow-symlinks"),
      QStringLiteral("-d"), QString::fromStdString(temp_path),
      QStringLiteral("-ef"), QString::fromStdString(list_path),
      QString::fromStdString(appimage_path)
    });
    p.start();
    return p.waitForFinished(15000) && p.exitCode() != 1;
  };
  auto try_stem_png = [&]() {
    std::string stem = fs::path(appimage_path).stem().string();
    fs::path stem_icon = fs::path(temp_path) / (stem + ".png");
    if (fs::is_regular_file(stem_icon)) {
      icon_file = stem_icon;
      return true;
    }
    if (!run_unsquashfs_ef(stem + ".png"))
      return false;
    if (fs::is_regular_file(stem_icon)) {
      icon_file = stem_icon;
      return true;
    }
    return false;
  };
  auto parse_icon_from_desktop = [](const fs::path& desktop_path) -> std::string {
    std::ifstream in(desktop_path);
    if (!in)
      return {};
    std::string line;
    while (std::getline(in, line)) {
      if (line.size() >= 5 && line.compare(0, 5, "Icon=") == 0) {
        std::string value(line.begin() + 5, line.end());
        std::size_t start = value.find_first_not_of(" \t");
        if (start != std::string::npos)
          value = value.substr(start);
        std::size_t end = value.find_last_not_of(" \t");
        if (end != std::string::npos)
          value = value.substr(0, end + 1);
        return value;
      }
    }
    return {};
  };
  auto try_desktop_icon = [&]() {
    QProcess list_proc;
    list_proc.setProgram(unsquashfs_path());
    list_proc.setArguments({
      QStringLiteral("-o"), QString::number(offset_opt.value()),
      QStringLiteral("-l"), QString::fromStdString(appimage_path)
    });
    list_proc.start();
    if (!list_proc.waitForFinished(10000))
      return false;
    QByteArray out = list_proc.readAllStandardOutput();
    std::string root_desktop;
    std::string applications_desktop;
    std::string fallback_desktop;
    for (const QByteArray& line : out.split('\n')) {
      std::string s = line.constData();
      while (!s.empty() && (s.back() == '\r' || s.back() == '\n'))
        s.pop_back();
      if (s.size() >= 8u && s.compare(s.size() - 8, 8, ".desktop") == 0) {
        std::size_t prefix = s.find("squashfs-root/");
        std::string candidate = prefix != std::string::npos ? s.substr(prefix + 14) : s;
        std::size_t path_start = candidate.find_first_not_of(" \t");
        if (path_start != std::string::npos) {
          std::size_t path_end = candidate.find_last_not_of(" \t");
          candidate = candidate.substr(path_start, path_end != std::string::npos ? path_end - path_start + 1 : std::string::npos);
        }
        if (candidate.find('/') == std::string::npos && root_desktop.empty())
          root_desktop = std::move(candidate);
        else if (candidate.find("usr/share/applications/") == 0u && applications_desktop.empty())
          applications_desktop = std::move(candidate);
        else if (fallback_desktop.empty())
          fallback_desktop = std::move(candidate);
      }
    }
    std::string desktop_rel_path = !root_desktop.empty() ? root_desktop : (!applications_desktop.empty() ? applications_desktop : fallback_desktop);
    if (desktop_rel_path.empty() || !run_unsquashfs_ef(desktop_rel_path))
      return false;
    fs::path desktop_path = fs::path(temp_path) / desktop_rel_path;
    std::string icon_value = parse_icon_from_desktop(desktop_path);
    if (icon_value.empty())
      return false;
    auto try_icon_path = [&](const std::string& rel_path) {
      if (!run_unsquashfs_ef(rel_path))
        return false;
      fs::path p = fs::path(temp_path) / rel_path;
      if (fs::is_regular_file(p)) {
        icon_file = p;
        return true;
      }
      return false;
    };
    if (icon_value.find('/') != std::string::npos) {
      std::string rel_path = icon_value;
      if (!rel_path.empty() && rel_path.front() == '/')
        rel_path.erase(0, 1);
      if (!rel_path.empty() && try_icon_path(rel_path))
        return true;
    }
    std::string icon_name = icon_value;
    std::size_t slash = icon_name.rfind('/');
    if (slash != std::string::npos)
      icon_name = icon_name.substr(slash + 1);
    std::size_t dot = icon_name.rfind('.');
    if (dot != std::string::npos && dot > 0)
      icon_name = icon_name.substr(0, dot);
    for (const auto& [prefix, ext] : icon_search_paths) {
      std::string rel = std::string(prefix) + icon_name + std::string(ext);
      if (try_icon_path(rel))
        return true;
    }
    return false;
  };
  if (!fs::is_regular_file(icon_file) && !try_stem_png() && !try_desktop_icon())
    return {};
  fs::path copy_src = icon_file;
  while (fs::is_symlink(copy_src)) {
    copy_src = copy_src.parent_path() / fs::read_symlink(copy_src);
  }
  if (!fs::is_regular_file(copy_src))
    return {};
  fs::create_directories(icons_dir);
  std::string ext = copy_src.extension().string();
  if (ext.empty())
    ext = ".png";
  std::string dest = application::icon_file_path(record_id, icons_dir, ext);
  std::error_code ec;
  fs::copy(copy_src, dest, fs::copy_options::overwrite_existing, ec);
  if (ec)
    return {};
  return dest;
}

}
