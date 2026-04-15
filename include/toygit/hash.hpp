#pragma once

#include <array>
#include <openssl/evp.h>
#include <stdexcept>
#include <utility>
#include <vector>

namespace toygit {
class Hasher {
  class MessageDigestContext {
    EVP_MD_CTX *mdc;

  public:
    MessageDigestContext() : mdc{EVP_MD_CTX_new()} {
      if (mdc == nullptr) {
        throw std::runtime_error{"Failed to obtain Message Digest Context"};
      }
    }
    MessageDigestContext(MessageDigestContext const &o) = delete;
    MessageDigestContext(MessageDigestContext &&o) noexcept : mdc{o.mdc} {
      o.mdc = nullptr;
    }
    MessageDigestContext &operator=(MessageDigestContext const &o) = delete;
    MessageDigestContext &operator=(MessageDigestContext &&o) noexcept {
      std::swap(mdc, o.mdc);
      return *this;
    }
    ~MessageDigestContext() {
      if (mdc != nullptr) {
        EVP_MD_CTX_free(mdc);
      }
    }
    [[nodiscard]] EVP_MD_CTX *get() const { return mdc; }
  };

  class MessageDigest {
    EVP_MD *md;

  public:
    explicit MessageDigest(EVP_MD *md) : md{md} {
      if (md == nullptr) {
        throw std::invalid_argument{"md == nullptr"};
      }
    }
    MessageDigest(MessageDigest const &o) = delete;
    MessageDigest(MessageDigest &&o) noexcept : md{o.md} { o.md = nullptr; }
    MessageDigest &operator=(MessageDigest const &o) = delete;
    MessageDigest &operator=(MessageDigest &&o) noexcept {
      std::swap(md, o.md);
      return *this;
    }
    ~MessageDigest() {
      if (md != nullptr) {
        EVP_MD_free(md);
      }
    }
    [[nodiscard]] EVP_MD *get() const { return md; }
  };

  MessageDigest md;
  MessageDigestContext mdc;

public:
  using Digest = std::vector<uint8_t>;

  explicit Hasher(EVP_MD *md) : md{md} {}

  Digest operator()(std::string_view message) {
    if (!EVP_DigestInit_ex(mdc.get(), md.get(), nullptr)) {
      throw std::runtime_error{"Failed to initialize message digest"};
    }
    if (!EVP_DigestUpdate(mdc.get(), message.data(), message.size())) {
      throw std::runtime_error{"Failed to update message digest"};
    }

    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int length{};
    if (!EVP_DigestFinal_ex(mdc.get(), digest.data(), &length)) {
      throw std::runtime_error{"Failed to extract message digest"};
    }
    return {digest.begin(), std::next(digest.begin(), length)};
  }

  template <size_t N> std::array<uint8_t, N> digest(std::string_view message) {
    if (!EVP_DigestInit_ex(mdc.get(), md.get(), nullptr)) {
      throw std::runtime_error{"Failed to initialize message digest"};
    }
    if (!EVP_DigestUpdate(mdc.get(), message.data(), message.size())) {
      throw std::runtime_error{"Failed to update message digest"};
    }

    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int length{};
    if (!EVP_DigestFinal_ex(mdc.get(), digest.data(), &length)) {
      throw std::runtime_error{"Failed to extract message digest"};
    }
    std::array<uint8_t, N> ret;
    std::copy(digest.begin(), std::next(digest.begin(), N), ret.begin());
    return ret;
  }

  static Hasher md5Hasher() {
    return Hasher{EVP_MD_fetch(nullptr, "MD5", nullptr)};
  }

  static Hasher sha1Hasher() {
    return Hasher{EVP_MD_fetch(nullptr, "SHA1", nullptr)};
  }
};
} // namespace toygit
