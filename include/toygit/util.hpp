#pragma once

#include <expected>
#include <functional>
#include <system_error>
#define TRY(...)                                                               \
  __extension__({                                                              \
    auto &&_try_r = (__VA_ARGS__);                                             \
    if (!_try_r) {                                                             \
      return std::unexpected{std::forward<decltype(_try_r)>(_try_r).error()};  \
    }                                                                          \
    *std::forward<decltype(_try_r)>(_try_r);                                   \
  })

#define FN(fn)                                                                 \
  [](auto &&...a) -> decltype(auto) {                                          \
    return fn(std::forward<decltype(a)>(a)...);                                \
  }

#define MFN(obj, fn)                                                           \
  [&](auto &&...a) -> decltype(auto) {                                         \
    return obj.fn(std::forward<decltype(a)>(a)...);                            \
  }

#define EC_CALL(fn, ...) ecCall(FN(fn) __VA_OPT__(, ) __VA_ARGS__)
#define TRY_EC(fn, ...) TRY(EC_CALL(fn __VA_OPT__(, ) __VA_ARGS__))
#define EC_CALL_M(obj, fn, ...) ecCall(MFN(obj, fn) __VA_OPT__(, ) __VA_ARGS__)
#define TRY_EC_M(obj, fn, ...)                                                 \
  TRY(EC_CALL_M(obj, fn __VA_OPT__(, ) __VA_ARGS__))

namespace toygit {

template <typename T> using Result = std::expected<T, std::error_code>;

template <typename F, typename... A> auto ecCall(F &&f, A &&...a) {
  using R =
      std::remove_cvref_t<std::invoke_result_t<F, A..., std::error_code &>>;
  using Out = std::expected<R, std::error_code>;
  auto ec = std::error_code{};
  if constexpr (std::is_void_v<R>) {
    std::invoke(std::forward<F>(f), std::forward<A>(a)..., ec);
    return ec ? Out{std::unexpect, ec} : Out{};
  } else {
    auto r = std::invoke(std::forward<F>(f), std::forward<A>(a)..., ec);
    return ec ? Out{std::unexpect, ec} : Out{std::move(r)};
  }
}

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
