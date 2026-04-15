#include "toygit/core.hpp"
#include "toygit/hash.hpp"
#include "toygit/zlib.hpp"
#include <filesystem>
#include <fstream>
#include <print>
#include <stdexcept>

namespace toygit {
constexpr auto DIGLEN = 20;

std::string hex(uint8_t byte) {
  static constexpr std::array<char, 16> bytes = {{'0', '1', '2', '3', '4', '5',
                                                  '6', '7', '8', '9', 'a', 'b',
                                                  'c', 'd', 'e', 'f'}};
  auto ret = std::string(2, 0);
  ret[0] = bytes[byte >> 4];
  ret[1] = bytes[byte & 0xF];
  return ret;
}

std::string hexString(std::span<uint8_t> bytes) {
  auto ret = std::string{};
  for (auto b : bytes) {
    ret.append(hex(b));
  }
  return ret;
}

void writeObject(std::string_view object, const std::filesystem::path &path) {
  auto ds = DeflateStream{};
  auto deflated = ds.deflate(object);
  auto ofs = std::ofstream{path};
  ofs << deflated;
}

void storeObject(std::string_view object) {
  auto hasher = Hasher::sha1Hasher();
  auto digest = hasher.digest<DIGLEN>(object);
  auto dir = std::filesystem::path{".git/objects"};
  dir.append(hex(digest[0]));
  std::filesystem::create_directories(dir);
  dir.append(hexString(std::span(digest).subspan(1)));
  switch (std::filesystem::status(dir).type()) {
    using enum std::filesystem::file_type;
  case regular:
    std::print("Object {} exists", hexString(digest));
    return;
  case not_found:
    std::print("Writing object {}", hexString(digest));
    return writeObject(object, dir);
  default:
    throw std::runtime_error{"DB object of unknown type"};
  }
}

void storeBlob(std::string_view blob) {
  auto data = std::string{"blob "};
  auto sz =
      std::array<char,
                 std::numeric_limits<std::string_view::size_type>::digits10>{};
  auto [ptr, ec] = std::to_chars(sz.begin(), sz.end(), blob.size());
  data.append(sz.begin(), ptr);
  data.append(1, 0);
  data.append(blob);
  std::println("{}", data);
  storeObject(data);
}

} // namespace toygit
