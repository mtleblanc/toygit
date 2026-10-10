#pragma once

#include <expected>
#include <system_error>
#define TRY(...)                                                               \
  __extension__({                                                              \
    auto &&_try_r = (__VA_ARGS__);                                             \
    if (!_try_r) {                                                             \
      return std::unexpected{std::move(_try_r.error())};                       \
    }                                                                          \
    *_try_r;                                                                   \
  })

namespace toygit {

template <typename T> using Result = std::expected<T, std::error_code>;
inline auto errnoCode() {
  return std::error_code{errno, std::system_category()};
}
inline auto errnoResult() { return std::unexpected{errnoCode()}; }

class NoMove {
public:
  NoMove() = default;
  NoMove(const NoMove &) = delete;
  NoMove(NoMove &&) = delete;
  NoMove &operator=(const NoMove &) = delete;
  NoMove &operator=(NoMove &&) = delete;
};

class MoveOnly {
public:
  MoveOnly() = default;
  MoveOnly(const MoveOnly &) = delete;
  MoveOnly(MoveOnly &&) = default;
  MoveOnly &operator=(const MoveOnly &) = delete;
  MoveOnly &operator=(MoveOnly &&) = default;
};
} // namespace toygit
