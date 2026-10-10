
#include "test_utils.hpp"
#include "toygit/add_command.hpp"
#include "toygit/dircache.hpp"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>

namespace toygit {
using namespace test;

TEST_CASE("Index", "[index]") {
  auto dir = test::TempDir{};
  fs::create_directory(dir / ".git");
  auto repository = std::make_shared<Repository>(dir, ".git");
  auto index = DirCache{repository};
  SECTION("adds a single file") {
    auto file = createFile(dir / "alice.txt", "alice");
    *index.add(file);
    REQUIRE(keys(index.entries()) == vs{"alice.txt"});
    REQUIRE(index.entries().at("alice.txt").header.modeString() == "100644");
  }

  SECTION("stores executable") {
    auto file = createFile(dir / "alice.txt", "alice");
    fs::permissions(file, fs::perms::owner_exec, fs::perm_options::add);
    *index.add(file);
    REQUIRE(keys(index.entries()) == vs{"alice.txt"});
    REQUIRE(index.entries().at("alice.txt").header.modeString() == "100755");
  }

  SECTION("replaces a file with a directory") {
    auto alice = createFile(dir / "alice.txt", "alice");
    auto bob = createFile(dir / "bob.txt", "bob");
    *index.add(alice);
    *index.add(bob);
    CHECK(keys(index.entries()) == vs{"alice.txt", "bob.txt"});
    fs::remove(alice);
    fs::create_directory(alice);
    auto aliceSub = createFile(alice / "sub_alice.txt", "alice sub");
    *index.add(aliceSub);
    REQUIRE(keys(index.entries()) == vs{"alice.txt/sub_alice.txt", "bob.txt"});
  }

  SECTION("replaces a directory with a file") {
    auto alice = dir / "alice.txt";
    auto bob = createFile(dir / "bob.txt", "bob");
    *index.add(bob);
    auto aliceSub1 = createFile(alice / "sub_alice.txt", "alice sub");
    auto aliceSub2 = createFile(alice / "sub_bob.txt", "bob sub");
    *index.add(aliceSub1);
    *index.add(aliceSub2);
    CHECK(keys(index.entries()) ==
          vs{"alice.txt/sub_alice.txt", "alice.txt/sub_bob.txt", "bob.txt"});
    fs::remove_all(alice);
    alice = createFile(alice, "alice");
    *index.add(alice);
    REQUIRE(keys(index.entries()) == vs{"alice.txt", "bob.txt"});
  }
}

TEST_CASE("Add command", "[add]") {
  auto dir = test::TempDir{};
  fs::create_directory(dir / ".git");
  auto repository = std::make_shared<Repository>(dir, ".git", dir);
  auto index = DirCache{repository};
  auto cmd = AddCommand{repository};
  SECTION("adds a single file") {
    auto file = createFile(dir / "alice.txt", "alice");
    auto args = vs{"alice.txt"};
    *cmd.run(args, {});
    *index.readFromFile();
    REQUIRE(keys(index.entries()) == vs{"alice.txt"});
    REQUIRE(index.entries().at("alice.txt").header.modeString() == "100644");
  }
}
} // namespace toygit
