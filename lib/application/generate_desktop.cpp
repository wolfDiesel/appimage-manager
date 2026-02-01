#include "generate_desktop.hpp"
#include <domain/entities/launch_settings.hpp>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>

namespace fs = std::filesystem;

namespace appimage_manager::application {

namespace {

std::string escape_desktop_string(const std::string& s) {
  std::string out;
  for (char c : s) {
    if (c == '\\' || c == ';' || c == '[' || c == ']' || c == '=' || c == '\n' || c == '\r')
      out += '\\';
    out += c;
  }
  return out;
}

std::string sanitize_id_for_filename(const std::string& id) {
  std::string out;
  for (char c : id) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_')
      out += c;
    else if (c == '/' || c == '\\')
      out += '-';
  }
  return out.empty() ? "app" : out;
}

std::string id_to_eight_digits(const std::string& id) {
  if (id.size() >= 8u)
    return id.substr(id.size() - 8);
  return std::string(8u - id.size(), '0') + id;
}

std::string build_exec_line(const domain::AppImageRecord& record,
                            const domain::LaunchSettings& settings) {
  std::string path_esc = escape_desktop_string(record.path);
  std::string args_esc = settings.args.empty() ? "" : " " + escape_desktop_string(settings.args);
  switch (settings.sandbox) {
    case domain::SandboxMechanism::Bwrap: {
      std::string bind = escape_desktop_string(record.path);
      return "bwrap --ro-bind " + bind + " " + bind + " --dev / -- " + path_esc + args_esc;
    }
    case domain::SandboxMechanism::Firejail:
      return "firejail -- " + path_esc + args_esc;
    default:
      return path_esc + args_esc;
  }
}

}

std::string desktop_file_path(const std::string& record_id,
                              const std::string& record_name,
                              const std::string& applications_dir) {
  std::string name_part = sanitize_id_for_filename(record_name);
  if (name_part.empty())
    name_part = "app";
  std::string digits = id_to_eight_digits(record_id);
  return (fs::path(applications_dir) / ("appimagemanager-" + name_part + "-" + digits + ".desktop")).string();
}

void generate_desktop(const domain::AppImageRecord& record,
                      const domain::LaunchSettings& settings,
                      const std::string& applications_dir,
                      const std::string& icon_path) {
  fs::path dir(applications_dir);
  if (!dir.empty())
    fs::create_directories(dir);
  std::string path = desktop_file_path(record.id, record.name, applications_dir);
  std::ofstream f(path);
  if (!f)
    return;
  f << "[Desktop Entry]\n";
  f << "Type=Application\n";
  f << "Name=" << escape_desktop_string(record.name) << "\n";
  f << "Exec=" << build_exec_line(record, settings) << "\n";
  if (!icon_path.empty())
    f << "Icon=" << escape_desktop_string(icon_path) << "\n";
  for (const auto& e : settings.env)
    f << "Env=" << escape_desktop_string(e) << "\n";
  f << "Terminal=false\n";
  f << "Categories=Utility;\n";
}

void remove_desktop(const std::string& record_id,
                    const std::string& record_name,
                    const std::string& applications_dir) {
  std::string path = desktop_file_path(record_id, record_name, applications_dir);
  std::error_code ec;
  fs::remove(path, ec);
}

std::string icon_file_path(const std::string& record_id,
                            const std::string& icons_dir,
                            const std::string& extension) {
  std::string base = sanitize_id_for_filename(record_id);
  std::string ext = extension.empty() || extension[0] != '.' ? "." + extension : extension;
  return (fs::path(icons_dir) / (base + ext)).string();
}

void remove_icon(const std::string& record_id,
                 const std::string& icons_dir) {
  std::error_code ec;
  fs::remove(icon_file_path(record_id, icons_dir, ".png"), ec);
  fs::remove(icon_file_path(record_id, icons_dir, ".svg"), ec);
}

}
