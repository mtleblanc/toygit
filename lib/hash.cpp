#include <toygit/hash.hpp>
namespace toygit {
Hasher<16> md5Hasher() {
  return Hasher<16>{EVP_MD_fetch(nullptr, "MD5", nullptr)};
}

Hasher<20> sha1Hasher() {
  return Hasher<20>{EVP_MD_fetch(nullptr, "SHA1", nullptr)};
}
} // namespace toygit
