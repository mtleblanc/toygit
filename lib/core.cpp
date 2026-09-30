#include "toygit/core.hpp"
#include "detail/object.hpp"
#include "toygit/hash.hpp"
#include "toygit/object.hpp"
#include "toygit/zlib.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <print>
#include <utility>

namespace toygit {

std::string hex(uint8_t byte) {
  auto ret = std::string(2, 0);
  std::format_to(ret.begin(), "{:02x}", byte);
  return ret;
}

std::string hexString(std::span<uint8_t> bytes) {
  auto ret = std::string{};
  ret.reserve(bytes.size() * 2);
  for (auto b : bytes) {
    ret.append(hex(b));
  }
  return ret;
}

void storeBlob(std::string_view blob) { Blob{blob}.store(); }

namespace {

void writeObject(std::string_view object, const std::filesystem::path &path) {
  auto deflated = DeflateStream::deflateOnce(object);
  auto ofs = std::ofstream{path};
  ofs << deflated;
}

} // namespace

Id Object::id() {
  auto hasher = sha1Hasher();
  return hasher.digest(content());
}

void Object::store() {
  auto digest = id();
  auto dir = std::filesystem::path{".toygit/objects"};
  dir.append(hex(digest[0]));
  std::filesystem::create_directories(dir);
  dir.append(hexString(std::span(digest).subspan(1)));
  switch (std::filesystem::status(dir).type()) {
    using enum std::filesystem::file_type;
  case regular:
    std::println("Object {} exists", hexString(digest));
    return;
  case not_found:
    std::println("Writing object {}", hexString(digest));
    return writeObject(content(), dir);
  default:
    throw std::runtime_error{"DB object of unknown type"};
  }
}

Blob::Blob(std::string_view text) : content_{packageContent("blob", text)} {}

std::string_view Blob::content() { return content_; }
std::string_view Blob::text() {
  auto endOfHeader = std::ranges::find(content_, 0);
  assert(endOfHeader != content_.end());
  return std::string_view{std::next(endOfHeader, 1), content_.end()};
}

std::shared_ptr<Blob> Blob::buildFrom(const std::filesystem::path &path) {
  auto ifs = std::ifstream{path};
  auto text = std::string{std::istreambuf_iterator<char>{ifs},
                          std::istreambuf_iterator<char>{}};
  return std::make_shared<Blob>(std::move(text));
};

std::shared_ptr<Blob> Blob::buildFromSymlink(std::filesystem::path path) {
  return std::make_shared<Blob>(std::filesystem::read_symlink(path).string());
};

std::string_view Commit::content() {
  if (!content_.empty()) {
    return content_;
  }
  std::string text{};
  text.append("tree ");
  text.append(hexString(tree_));
  text.append("\n");
  if (parent_) {
    text.append("parent ");
    text.append(*parent_);
    text.append("\n");
  }
  text.append("author ");
  text.append(author_);
  text.append(" ");
  text.append(std::format("{}", timestamp_));
  text.append("\n");
  text.append("committer ");
  text.append(author_);
  text.append(" ");
  text.append(std::format("{}", timestamp_));
  text.append("\n");
  text.append("\n");
  text.append(message_);
  text.append("\n");
  return content_ = packageContent("commit", text);
}
} // namespace toygit
