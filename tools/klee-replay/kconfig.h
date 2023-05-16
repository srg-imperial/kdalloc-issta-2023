#pragma once

#include <string>
#include <filesystem>

struct KConfig {
  bool kdalloc = false;
  std::string kdalloc_quarantine;
  std::string kdalloc_heap_start_address;
  std::string kdalloc_heap_size;

  KConfig(std::filesystem::path const& path);
};