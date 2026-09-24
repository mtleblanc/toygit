#include "toygit/dircache.hpp"
#include "toygit/lockfile.hpp"
#include <arpa/inet.h>
#include <cassert>
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

static constexpr size_t ENTRY_HEADER_SIZE = sizeof(DirCache::EntryHeader) - 2;
static constexpr size_t ENTRY_ALIGNMENT = 8;

DirCache::EntryHeader entryFrom(std::byte *buf) {
  static_assert(sizeof(DirCache::EntryHeader) == 64);
  static_assert(offsetof(DirCache::EntryHeader, ctimeSeconds) == 0);
  static_assert(offsetof(DirCache::EntryHeader, ctimeNanos) == 4);
  static_assert(offsetof(DirCache::EntryHeader, mtimeSeconds) == 8);
  static_assert(offsetof(DirCache::EntryHeader, mtimeNanos) == 12);
  static_assert(offsetof(DirCache::EntryHeader, device) == 16);
  static_assert(offsetof(DirCache::EntryHeader, inode) == 20);
  static_assert(offsetof(DirCache::EntryHeader, mdoe) == 24);
  static_assert(offsetof(DirCache::EntryHeader, uid) == 28);
  static_assert(offsetof(DirCache::EntryHeader, gid) == 32);
  static_assert(offsetof(DirCache::EntryHeader, size) == 36);
  static_assert(offsetof(DirCache::EntryHeader, id) == 40);
  static_assert(offsetof(DirCache::EntryHeader, flags) == 60);
  DirCache::EntryHeader h;
  std::memcpy(&h, buf, sizeof(DirCache::EntryHeader));
  h.ctimeSeconds = ::ntohl(h.ctimeSeconds);
  h.ctimeNanos = ::ntohl(h.ctimeNanos);
  h.mtimeSeconds = ::ntohl(h.mtimeSeconds);
  h.mtimeNanos = ::ntohl(h.mtimeNanos);
  h.device = ::ntohl(h.device);
  h.inode = ::ntohl(h.inode);
  h.mdoe = ::ntohl(h.mdoe);
  h.uid = ::ntohl(h.uid);
  h.gid = ::ntohl(h.gid);
  h.size = ::ntohl(h.size);
  h.flags = ::ntohs(h.flags);
  return h;
}

} // namespace

static const auto dir = std::filesystem::path{".toygit/index"};

DirCache DirCache::readFromFile() {
  auto lf = Lockfile{dir, true};
  std::byte buf[1024];
  static_assert(sizeof(buf) >= sizeof(Header));
  auto dst = std::span{buf, sizeof(Header)};
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

  auto res = lf.read({buf, sizeof(buf)});
  if (!res) {
    throw std::system_error{res.error(), "Could not read index"};
  }
  // should at least be the hash even if entries == 0
  if (res.value() == 0) {
    throw std::runtime_error{"Index file reached EOF before footer"};
  }
  auto valid = std::span{buf, static_cast<size_t>(res.value())};
  auto cache = DirCache{};
  cache.version = header.version;
  for (; header.entries > 0; --header.entries) {
    if (valid.size() < sizeof(DirCache::EntryHeader)) {
      static_assert(sizeof(DirCache::EntryHeader) <= sizeof(buf));
      std::memmove(buf, valid.data(), valid.size());
      valid = {buf, valid.size()};
      while (valid.size() < sizeof(DirCache::EntryHeader)) {
        res = lf.read({buf + valid.size(), sizeof(buf) - valid.size()});
        if (!res) {
          throw std::system_error{res.error(), "Could not read index"};
        }
        if (res.value() == 0) {
          throw std::runtime_error{"Index file reached EOF mid entry"};
        }
        valid = {buf, valid.size() + res.value()};
      }
    }
    auto entryHeader = entryFrom(valid.data());
    valid = valid.subspan(ENTRY_HEADER_SIZE);
    auto sz = (size_t)entryHeader.flags & 0xFFF;
    auto name = std::string{};
    auto alignment = ENTRY_HEADER_SIZE;
    while (name.size() < sz) {
      auto toCopy = std::min(sz - name.size(), valid.size());
      auto sourceBytes = valid.first(toCopy);
      auto source = std::span{reinterpret_cast<char *>(sourceBytes.data()),
                              sourceBytes.size()};
      name.append(source.begin(), source.end());
      alignment += source.size();
      valid = valid.subspan(source.size());
      if (valid.size() == 0) {
        res = lf.read({buf, sizeof(buf)});

        if (!res) {
          throw std::system_error{res.error(), "Could not read index"};
        }
        if (res.value() == 0) {
          throw std::runtime_error{"Index file reached EOF mid entry"};
        }
        valid = {buf, (size_t)res.value()};
      }
    }
    assert(valid.size() != 0);
    // TODO: handle size > 0xFFF
    if (valid.front() != std::byte{0}) {
      throw std::runtime_error("Non null terminated name in index");
    }
    auto padding = ENTRY_ALIGNMENT - (alignment % ENTRY_ALIGNMENT);
    while (valid.size() < padding) {
      padding -= valid.size();
      res = lf.read({buf, sizeof(buf)});

      if (!res) {
        throw std::system_error{res.error(), "Could not read index"};
      }
      if (res.value() == 0) {
        throw std::runtime_error{"Index file reached EOF mid entry"};
      }
      valid = {buf, (size_t)res.value()};
    }
    // TODO:: Should we check padding bytes are all 0?
    valid = valid.subspan(padding);
    std::println("Added entry for {}", name);
    cache.entries.emplace_back(entryHeader, name);
  }
  // TODO: Should we check checksum?
  return cache;
}
} // namespace toygit
