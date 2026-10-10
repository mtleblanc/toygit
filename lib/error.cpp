#include "toygit/error.hpp"

namespace toygit {
const GitErrorCategory cat{};

std::error_code make_error_code(GitError e) {
  return std::error_code{static_cast<int>(e), cat};
};

std::string GitErrorCategory::message(int num) const {

  switch (static_cast<GitError>(num)) {
  case GitError::NONE:
    return std::string{"Success"};
  case GitError::ERROR:
    return std::string{"error: "};
  case GitError::FATAL:
    return std::string{"fatal: "};
  default:
    return std::string{"unknown error: "};
  }
}

bool isGitError(std::error_code err) { return err.category() == cat; }

bool isFailure(std::error_code err) {
  return err.category() == cat &&
         err.value() == static_cast<int>(GitError::FATAL);
}
bool isFatal(std::error_code err) {
  return err.category() == cat &&
         err.value() != static_cast<int>(GitError::NONE);
}
} // namespace toygit
