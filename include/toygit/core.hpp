#pragma once

#include "toygit/object.hpp"
#include <cstdint>
#include <expected>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace toygit {

void storeObject(std::string_view object);
void storeBlob(std::string_view blob);
std::string hexString(std::span<uint8_t>);

class Blob : public Object {
public:
  Blob(std::string_view text);

  std::string_view content() override;

  std::string_view text();

  static std::shared_ptr<Blob> buildFrom(const std::filesystem::path &);
  static std::shared_ptr<Blob> buildFromSymlink(const std::filesystem::path &);

private:
  std::string content_;
};

class Commit : public Object {
public:
  std::string_view content() override;

  Commit(Id tree, std::optional<std::string> parent, std::string author,
         int64_t timestamp, std::string message)
      : tree_{tree}, parent_{std::move(parent)}, author_{std::move(author)},
        timestamp_{timestamp}, message_{std::move(message)} {}

private:
  std::string content_;
  Id tree_;
  std::optional<std::string> parent_;
  std::string author_;
  int64_t timestamp_;
  std::string message_;
};
} // namespace toygit
