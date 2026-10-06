#pragma once

#include <system_error>
#include <type_traits>
namespace toygit {
enum class GitError { NONE = 0, ERROR, FATAL };

struct GitErrorCategory : std::error_category {
  const char *name() const noexcept override { return "toygit"; }
  std::string message(int num) const override;
};
std::error_code make_error_code(GitError e);
bool isFailure(std::error_code err);
bool isFatal(std::error_code err);
} // namespace toygit

namespace std {
template <> struct is_error_code_enum<toygit::GitError> : std::true_type {};
} // namespace std
