#include "object.hpp"
#include <array>
#include <cassert>
#include <charconv>
#include <limits>
#include <system_error>

namespace toygit {
std::string packageContent(std::string_view type, std::string_view text) {
  static constexpr auto MAX_DIGITS =
      std::numeric_limits<std::string_view::size_type>::digits10;
  auto data = std::string{type};
  data.append(" ");
  auto sz = std::array<char, MAX_DIGITS>{};
  auto [ptr, ec] = std::to_chars(sz.begin(), sz.end(), text.size());
  assert(ec != std::errc::value_too_large);
  data.append(sz.begin(), ptr);
  data.append(1, 0);
  data.append(text);
  return data;
}
} // namespace toygit
