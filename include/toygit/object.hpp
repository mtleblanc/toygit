#pragma once

#include "toygit/util.hpp"
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

  static Result<std::shared_ptr<Object>> load();
};
} // namespace toygit
