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

private:
  std::map<std::string, Id> children_;
  std::string content_;
};

} // namespace toygit
