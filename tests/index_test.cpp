
#include "test_utils.hpp"
#include "toygit/dircache.hpp"
#include <catch2/catch_test_macros.hpp>
#include <fstream>

namespace toygit {
using namespace test;

TEST_CASE("Adding single entry to index", "[index]") {
  auto dir = test::TempDir{};
  fs::create_directory(dir / ".git"_p);
  auto repository = std::make_shared<Repository>(dir, ".git");
  auto index = DirCache{repository};
  SECTION("adding single file") {
    auto file = dir / "alice.txt"_p;
    std::ofstream{file} << "alice";
    *index.add(file);
    REQUIRE(keys(index.entries()) == vs{"alice.txt"});
  }

  SECTION("replaces a file with a directory") {
    auto aliceFile = dir / "alice.txt"_p;
    auto bobFile = dir / "bob.txt"_p;
    std::ofstream{aliceFile} << "alice";
    std::ofstream{bobFile} << "bob";
    *index.add(aliceFile);
    *index.add(bobFile);
    fs::remove(aliceFile);
    fs::create_directory(aliceFile);
    auto fileInAliceDir = aliceFile / "sub_alice.txt";
    std::ofstream{fileInAliceDir} << "alice sub";
    *index.add(fileInAliceDir);
    REQUIRE(keys(index.entries()) == vs{"alice.txt/sub_alice.txt", "bob.txt"});
  }
}
} // namespace toygit
