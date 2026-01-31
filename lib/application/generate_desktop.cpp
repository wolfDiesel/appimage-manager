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
                              const std::string& applications_dir) {
  std::string base = sanitize_id_for_filename(record_id);
  return (fs::path(applications_dir) / ("appimage-manager-" + base + ".desktop")).string();
}

void generate_desktop(const domain::AppImageRecord& record,
                      const domain::LaunchSettings& settings,
                      const std::string& applications_dir) {
  fs::path dir(applications_dir);
  if (!dir.empty())
    fs::create_directories(dir);
  std::string path = desktop_file_path(record.id, applications_dir);
  std::ofstream f(path);
  if (!f)
    return;
  f << "[Desktop Entry]\n";
  f << "Type=Application\n";
  f << "Name=" << escape_desktop_string(record.name) << "\n";
  f << "Exec=" << build_exec_line(record, settings) << "\n";
  for (const auto& e : settings.env)
    f << "Env=" << escape_desktop_string(e) << "\n";
  f << "Terminal=false\n";
  f << "Categories=Utility;\n";
}

void remove_desktop(const std::string& record_id,
                   const std::string& applications_dir) {
  std::string path = desktop_file_path(record_id, applications_dir);
  std::error_code ec;
  fs::remove(path, ec);
}

}
