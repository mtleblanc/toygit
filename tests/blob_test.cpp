#include "toygit/core.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Blob id matches git's own hash for known content", "[blob]") {
  auto blob = toygit::Blob{"hello\n"};
  auto id = blob.id();

  // Cross-checked independently: printf 'blob 6\0hello\n' | sha1sum
  REQUIRE(toygit::hexString(std::span{id}) ==
          "ce013625030ba8dba906f756967f9e9ca394464a");
}
