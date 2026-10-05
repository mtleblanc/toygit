#include "toygit/repository.hpp"

namespace toygit {

std::filesystem::path Repository::gitPath(std::string_view path) const {
  return git_ / path;
}

const std::filesystem::path &Repository::root() const { return projectRoot_; }

bool Repository::shouldIgnore(const std::filesystem::path &path,
                              struct stat &stat) const {

  if (S_ISREG(stat.st_mode) || S_ISLNK(stat.st_mode)) {
    return false;
  }
  if (S_ISDIR(stat.st_mode)) {
    auto filename = path.filename();
    if (filename == "build" || filename == ".cache" || filename == ".git" ||
        filename == ".toygit") {
      return true;
    }
    return false;
  }
  std::println("Skipping {}", path.c_str());
  return true;
}

bool Repository::shouldIgnore(
    const std::filesystem::directory_entry &path) const {

  if (path.is_regular_file()) {
    return false;
  }
  if (path.is_directory()) {
    auto filename = path.path().filename();
    if (filename == "build" || filename == ".cache" || filename == ".git" ||
        filename == ".toygit") {
      return true;
    }
    return false;
  }
  return true;
}
} // namespace toygit
