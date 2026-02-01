#include "extract_icon.hpp"
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <algorithm>
#include <array>

namespace fs = std::filesystem;

namespace appimage_manager::application {

namespace {

constexpr std::uint8_t ELF_MAGIC[] = {0x7f, 'E', 'L', 'F'};
constexpr std::uint8_t SQUASHFS_MAGIC[] = {'h', 's', 'q', 's'};
constexpr std::uint32_t PT_LOAD = 1;

std::optional<std::uint64_t> get_elf64_load_end(std::ifstream& f) {
  std::uint64_t e_phoff;
  std::uint16_t e_phnum, e_phentsize;
  f.seekg(0x20);
  if (!f.read(reinterpret_cast<char*>(&e_phoff), sizeof(e_phoff)))
    return std::nullopt;
  f.seekg(0x36);
  if (!f.read(reinterpret_cast<char*>(&e_phentsize), sizeof(e_phentsize)) ||
      !f.read(reinterpret_cast<char*>(&e_phnum), sizeof(e_phnum)))
    return std::nullopt;
  if (e_phnum == 0 || e_phentsize < 56)
    return std::nullopt;
  std::uint64_t end_offset = 0;
  for (std::uint16_t i = 0; i < e_phnum; ++i) {
    f.seekg(e_phoff + static_cast<std::uint64_t>(i) * e_phentsize);
    std::uint32_t p_type;
    std::uint64_t p_offset, p_filesz;
    if (!f.read(reinterpret_cast<char*>(&p_type), sizeof(p_type)))
      return std::nullopt;
    f.seekg(e_phoff + static_cast<std::uint64_t>(i) * e_phentsize + 8);
    if (!f.read(reinterpret_cast<char*>(&p_offset), sizeof(p_offset)))
      return std::nullopt;
    f.seekg(e_phoff + static_cast<std::uint64_t>(i) * e_phentsize + 32);
    if (!f.read(reinterpret_cast<char*>(&p_filesz), sizeof(p_filesz)))
      return std::nullopt;
    if (p_type == PT_LOAD)
      end_offset = std::max(end_offset, p_offset + p_filesz);
  }
  return end_offset;
}

std::optional<std::uint64_t> get_elf32_load_end(std::ifstream& f) {
  std::uint32_t e_phoff;
  std::uint16_t e_phnum, e_phentsize;
  f.seekg(0x1c);
  if (!f.read(reinterpret_cast<char*>(&e_phoff), sizeof(e_phoff)))
    return std::nullopt;
  f.seekg(0x2a);
  if (!f.read(reinterpret_cast<char*>(&e_phentsize), sizeof(e_phentsize)) ||
      !f.read(reinterpret_cast<char*>(&e_phnum), sizeof(e_phnum)))
    return std::nullopt;
  if (e_phnum == 0 || e_phentsize < 32)
    return std::nullopt;
  std::uint64_t end_offset = 0;
  for (std::uint16_t i = 0; i < e_phnum; ++i) {
    f.seekg(e_phoff + static_cast<std::uint64_t>(i) * e_phentsize);
    std::uint32_t p_type, p_offset, p_filesz;
    if (!f.read(reinterpret_cast<char*>(&p_type), sizeof(p_type)) ||
        !f.read(reinterpret_cast<char*>(&p_offset), sizeof(p_offset)))
      return std::nullopt;
    f.seekg(e_phoff + static_cast<std::uint64_t>(i) * e_phentsize + 16);
    if (!f.read(reinterpret_cast<char*>(&p_filesz), sizeof(p_filesz)))
      return std::nullopt;
    if (p_type == PT_LOAD)
      end_offset = std::max(end_offset, static_cast<std::uint64_t>(p_offset) + p_filesz);
  }
  return end_offset;
}

std::optional<std::uint64_t> find_squashfs_magic(std::ifstream& f, std::uint64_t start) {
  f.seekg(start);
  std::array<char, 65536> buf;
  std::uint64_t pos = start;
  while (f.read(buf.data(), buf.size()) || f.gcount() > 0) {
    std::streamsize n = f.gcount();
    for (std::streamsize i = 0; i + 4 <= n; ++i) {
      if (buf[static_cast<std::size_t>(i)] == SQUASHFS_MAGIC[0] &&
          buf[static_cast<std::size_t>(i) + 1] == SQUASHFS_MAGIC[1] &&
          buf[static_cast<std::size_t>(i) + 2] == SQUASHFS_MAGIC[2] &&
          buf[static_cast<std::size_t>(i) + 3] == SQUASHFS_MAGIC[3])
        return pos + static_cast<std::uint64_t>(i);
    }
    pos += static_cast<std::uint64_t>(n);
    if (static_cast<std::streamsize>(n) < buf.size())
      break;
  }
  return std::nullopt;
}

}

std::optional<std::uint64_t> get_appimage_squashfs_offset(const std::string& appimage_path) {
  std::ifstream f(appimage_path, std::ios::binary);
  if (!f)
    return std::nullopt;
  std::uint8_t magic[4];
  if (!f.read(reinterpret_cast<char*>(magic), 4) ||
      !std::equal(std::begin(ELF_MAGIC), std::end(ELF_MAGIC), magic))
    return std::nullopt;
  std::uint8_t ei_class;
  if (!f.read(reinterpret_cast<char*>(&ei_class), 1))
    return std::nullopt;
  std::optional<std::uint64_t> load_end;
  if (ei_class == 2)
    load_end = get_elf64_load_end(f);
  else if (ei_class == 1)
    load_end = get_elf32_load_end(f);
  else
    return std::nullopt;
  if (!load_end.has_value())
    return std::nullopt;
  return find_squashfs_magic(f, load_end.value());
}

}
