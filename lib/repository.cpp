#include "toygit/repository.hpp"

namespace toygit {

std::filesystem::path Repository::gitPath(std::string_view path) const {
  return git_ / path;
}

const std::filesystem::path &Repository::root() const { return projectRoot_; }
} // namespace toygit
