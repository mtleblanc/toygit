#include "toygit/dircache.hpp"
#include "toygit/lockfile.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <print>
#include <stdexcept>

namespace toygit {

namespace {
struct Header {
  int32_t signature;
  int32_t version;
  int32_t entries;

  static constexpr int32_t SIGNATURE = 0x44495243; // "DIRC"
  static constexpr int32_t MIN_VERSION = 2;
  static constexpr int32_t MAX_VERSION = 2;

  static Header from(std::byte *buf) {
    static_assert(sizeof(Header) == 12);
    static_assert(offsetof(Header, signature) == 0);
    static_assert(offsetof(Header, version) == 4);
    static_assert(offsetof(Header, entries) == 8);
    Header h;
    std::memcpy(&h, buf, sizeof(Header));
    h.signature = ::ntohl(h.signature);
    h.version = ::ntohl(h.version);
    h.entries = ::ntohl(h.entries);
    return h;
  }

  constexpr bool isValid() const {
    return signature == SIGNATURE && version >= MIN_VERSION &&
           version <= MAX_VERSION;
  }
};

} // namespace

static const auto dir = std::filesystem::path{".toygit/index"};

DirCache DirCache::readFromFile() {
  auto lf = Lockfile{dir, true};
  std::byte buf[1024];
  auto dst = std::span{buf, 12};
  do {
    auto res = lf.read(dst);
    if (!res) {
      throw std::system_error{res.error(), "Could not read index"};
    }
    if (res.value() == 0) {
      throw std::runtime_error{"Index file reached EOF before header"};
    }
    dst = dst.subspan(res.value());
  } while (!dst.empty());

  auto header = Header::from(buf);
  if (!header.isValid()) {
    throw std::runtime_error{"Invalid index header"};
  }

  std::println("Index has {} entries", header.entries);
  return DirCache{};
}
} // namespace toygit
