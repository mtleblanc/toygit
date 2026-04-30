#include "toygit/core.hpp"
#include "toygit/hash.hpp"
#include "toygit/zlib.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <print>
#include <stdexcept>
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
} // namespace

Id Object::id() {
  auto hasher = Hasher::sha1Hasher();
  return hasher.digest<std::tuple_size_v<Id>>(content());
}

void Object::store() {
  auto digest = id();
  auto dir = std::filesystem::path{".git/objects"};
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

std::shared_ptr<Blob> Blob::buildFrom(std::filesystem::path path) {
  auto ifs = std::ifstream{path};
  auto text = std::string{std::istreambuf_iterator<char>{ifs},
                          std::istreambuf_iterator<char>{}};
  return std::make_shared<Blob>(std::move(text));
};

namespace {
const std::string &modeString(Tree::Mode m) {
  static auto DIR = std::string{"40000"};
  static auto REGULAR = std::string{"100644"};
  static auto EXECUTABLE = std::string{"100755"};
  switch (m) {
  case Tree::Mode::DIRECTORY:
    return DIR;
  case Tree::Mode::REGUALAR_FILE:
    return REGULAR;
  case Tree::Mode::EXECUTABLE_FILE:
    return EXECUTABLE;
  default:
    std::unreachable();
  }
}
} // namespace

std::string_view Tree::content() {
  if (!content_.empty()) {
    return content_;
  }
  std::string text{};
  for (const auto &[name, idMode] : children_) {
    auto [id, mode] = idMode;
    text.append(modeString(mode));
    text.append(" ");
    text.append(name);
    text.append(1, 0);
    text.append(id.begin(), id.end());
  }
  return content_ = packageContent("tree", text);
}

std::shared_ptr<Tree> Tree::buildFrom(std::filesystem::path path) {
  namespace fs = std::filesystem;
  auto ec = std::error_code{};
  auto status = fs::status(path, ec);
  if (status.type() != fs::file_type::directory) {
    return {};
  }
  auto res = std::make_shared<Tree>();
  for (auto &de : fs::directory_iterator(path)) {
    if (de.is_directory() && de.path().filename() != ".git" &&
        de.path().filename() != "build" && de.path().filename() != ".cache") {
      auto thisPath = de.path();
      auto entry = buildFrom(thisPath);
      if (entry) {
        entry->store();
        res->children_[thisPath.filename()] =
            std::make_tuple(entry->id(), Tree::Mode::DIRECTORY);
      }
    }
    if (de.is_regular_file()) {
      auto thisPath = de.path();
      auto entry = Blob::buildFrom(thisPath);
      if (entry) {
        entry->store();
        auto isExecutable = (de.status().permissions() &
                             fs::perms::owner_exec) == fs::perms::none;
        res->children_[thisPath.filename()] = std::make_tuple(
            entry->id(), isExecutable ? Tree::Mode::EXECUTABLE_FILE
                                      : Tree::Mode::REGUALAR_FILE);
      }
    }
  }
  return res;
}

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
