#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <memory>
#include <string_view>

namespace toygit {

using Id = std::array<uint8_t, 20>;
class Object {
public:
  virtual ~Object() = default;
  virtual std::string_view content() = 0;

  Id id();
  void store();

  static std::expected<std::shared_ptr<Object>, std::error_code> load(Id);
};
} // namespace toygit
