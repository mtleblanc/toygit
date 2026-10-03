
#include "test_utils.hpp"
#include "toygit/dircache.hpp"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

namespace toygit {
using namespace test;

TEST_CASE("Index", "[index]") {
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
    CHECK(keys(index.entries()) == vs{"alice.txt", "bob.txt"});
    fs::remove(aliceFile);
    fs::create_directory(aliceFile);
    auto fileInAliceDir = aliceFile / "sub_alice.txt";
    std::ofstream{fileInAliceDir} << "alice sub";
    *index.add(fileInAliceDir);
    REQUIRE(keys(index.entries()) == vs{"alice.txt/sub_alice.txt", "bob.txt"});
  }

  SECTION("replaces a directory with a file") {
    auto aliceFile = dir / "alice.txt"_p;
    auto bobFile = dir / "bob.txt"_p;
    std::ofstream{bobFile} << "bob";
    *index.add(bobFile);
    fs::create_directory(aliceFile);
    auto file1InAliceDir = aliceFile / "sub_alice.txt";
    auto file2InAliceDir = aliceFile / "sub_bob.txt";
    std::ofstream{file1InAliceDir} << "alice sub";
    std::ofstream{file2InAliceDir} << "bob sub";
    *index.add(file1InAliceDir);
    *index.add(file2InAliceDir);
    CHECK(keys(index.entries()) ==
          vs{"alice.txt/sub_alice.txt", "alice.txt/sub_bob.txt", "bob.txt"});
    fs::remove_all(aliceFile);
    std::ofstream{aliceFile} << "alice";
    *index.add(aliceFile);
    REQUIRE(keys(index.entries()) == vs{"alice.txt", "bob.txt"});
  }
}
} // namespace toygit
