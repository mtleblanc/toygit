#pragma once

#include <filesystem>
#include <print>
#include <sys/stat.h>

namespace toygit {

class Repository {
public:
  Repository(std::filesystem::path root, std::filesystem::path git,
             std::filesystem::path cwd = std::filesystem::current_path())
      : projectRoot_{std::move(root)}, git_{projectRoot_ / std::move(git)},
        cwd_{std::move(cwd)} {}

  std::filesystem::path gitPath(std::string_view path) const {
    return git_ / path;
  };
  const std::filesystem::path &root() const { return projectRoot_; };
  const std::filesystem::path &cwd() const { return cwd_; };
  bool shouldIgnore(const std::filesystem::path &path, struct stat &stat) const;
  bool shouldIgnore(const std::filesystem::directory_entry &de) const;

private:
  std::filesystem::path projectRoot_;
  std::filesystem::path git_;
  std::filesystem::path cwd_;
};
} // namespace toygit
