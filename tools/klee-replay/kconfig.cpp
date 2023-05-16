#include "kconfig.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <regex>

KConfig::KConfig(std::filesystem::path const &path) {
  static std::regex line_re(R"(^([^:]+):([^\n]+)$)");

  std::ifstream file(path);
  std::string line;
  while (std::getline(file, line)) {
    if (!line.empty()) {
      std::smatch match;
      if (!std::regex_match(line, match, line_re)) {
        std::cerr << "KLEE-REPLAY: ERROR: kconfig file " << path
                  << " not valid.\n";
        std::abort();
      }

      if (match[1] == "kdalloc") {
        if (match[2] == "0") {
          kdalloc = false;
        } else if (match[2] == "1") {
          kdalloc = true;
        } else {
          std::cerr << "KLEE-REPLAY: ERROR: kconfig file " << path
                    << " invalid value for key \"kdalloc\".\n";
          std::abort();
        }
      } else if (match[1] == "kdalloc-quarantine") {
        kdalloc_quarantine = match[2];
      } else if (match[1] == "kdalloc-heap-start-address") {
        kdalloc_heap_start_address = match[2];
      } else if (match[1] == "kdalloc-heap-size") {
        kdalloc_heap_size = match[2];
      }
    }
  }
}