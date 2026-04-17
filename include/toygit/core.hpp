#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>

namespace toygit {

using Id = std::array<uint8_t, 20>;

void storeObject(std::string_view object);
void storeBlob(std::string_view blob);
std::string hexString(std::span<uint8_t>);

class Object {
public:
  virtual ~Object() = default;
  virtual std::string_view content() = 0;

  Id id();
  void store();

  static std::expected<std::shared_ptr<Object>, std::error_code> load(Id);
};

class Blob : public Object {
public:
  Blob(std::string_view text);

  std::string_view content() override;

  std::string_view text();

  static std::shared_ptr<Blob> buildFrom(std::filesystem::path);

private:
  std::string content_;
};

class Tree : public Object {
public:
  std::string_view content() override;
  static std::shared_ptr<Tree> buildFrom(std::filesystem::path);

  using Mode = std::array<char, 6>;

private:
  std::map<std::string, std::tuple<Id, Mode>> children_;
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
