#pragma once

#include <filesystem>
#include <print>
#include <sys/stat.h>

namespace toygit {

class Repository {
public:
  Repository(std::filesystem::path root, std::filesystem::path git)
      : projectRoot_{std::move(root)}, git_{projectRoot_ / std::move(git)} {}

  std::filesystem::path gitPath(std::string_view path) const;
  const std::filesystem::path &root() const;
  bool shouldIgnore(const std::filesystem::path &path, struct stat &stat) const;
  bool shouldIgnore(const std::filesystem::directory_entry &de) const;

private:
  std::filesystem::path projectRoot_;
  std::filesystem::path git_;
};
} // namespace toygit
