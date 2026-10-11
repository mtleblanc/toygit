
#include "test_utils.hpp"
#include "toygit/add_command.hpp"
#include "toygit/dircache.hpp"
#include "toygit/error.hpp"
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
    CHECK(index.add(file));
    REQUIRE(keys(index.entries()) == vs{"alice.txt"});
    REQUIRE(index.entries().at("alice.txt").header.modeString() == "100644");
  }

  SECTION("stores executable") {
    auto file = createFile(dir / "alice.txt", "alice");
    fs::permissions(file, fs::perms::owner_exec, fs::perm_options::add);
    CHECK(index.add(file));
    REQUIRE(keys(index.entries()) == vs{"alice.txt"});
    REQUIRE(index.entries().at("alice.txt").header.modeString() == "100755");
  }

  SECTION("replaces a file with a directory") {
    auto alice = createFile(dir / "alice.txt", "alice");
    auto bob = createFile(dir / "bob.txt", "bob");
    CHECK(index.add(alice));
    CHECK(index.add(bob));
    CHECK(keys(index.entries()) == vs{"alice.txt", "bob.txt"});
    fs::remove(alice);
    fs::create_directory(alice);
    auto aliceSub = createFile(alice / "sub_alice.txt", "alice sub");
    CHECK(index.add(aliceSub));
    REQUIRE(keys(index.entries()) == vs{"alice.txt/sub_alice.txt", "bob.txt"});
  }

  SECTION("replaces a directory with a file") {
    auto alice = dir / "alice.txt";
    auto bob = createFile(dir / "bob.txt", "bob");
    CHECK(index.add(bob));
    auto aliceSub1 = createFile(alice / "sub_alice.txt", "alice sub");
    auto aliceSub2 = createFile(alice / "sub_bob.txt", "bob sub");
    CHECK(index.add(aliceSub1));
    CHECK(index.add(aliceSub2));
    CHECK(keys(index.entries()) ==
          vs{"alice.txt/sub_alice.txt", "alice.txt/sub_bob.txt", "bob.txt"});
    fs::remove_all(alice);
    alice = createFile(alice, "alice");
    CHECK(index.add(alice));
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
    CHECK(cmd.run(args, {}));
    CHECK(index.readFromFile());
    REQUIRE(keys(index.entries()) == vs{"alice.txt"});
    REQUIRE(index.entries().at("alice.txt").header.modeString() == "100644");
  }
  SECTION("adds multiple files") {
    auto alice = createFile(dir / "alice.txt", "alice");
    auto bob = createFile(dir / "bob.txt", "bob");
    auto args = vs{"alice.txt", "bob.txt"};
    CHECK(cmd.run(args, {}));
    CHECK(index.readFromFile());
    REQUIRE(keys(index.entries()) == vs{"alice.txt", "bob.txt"});
    REQUIRE(index.entries().at("alice.txt").header.modeString() == "100644");
    REQUIRE(index.entries().at("bob.txt").header.modeString() == "100644");
  }
  SECTION("adds files incrementally") {
    auto alice = createFile(dir / "alice.txt", "alice");
    auto bob = createFile(dir / "bob.txt", "bob");
    auto args = vs{"alice.txt"};
    CHECK(cmd.run(args, {}));
    CHECK(index.readFromFile());
    REQUIRE(keys(index.entries()) == vs{"alice.txt"});
    REQUIRE(index.entries().at("alice.txt").header.modeString() == "100644");
    args = vs{"bob.txt"};
    CHECK(cmd.run(args, {}));
    CHECK(index.readFromFile());
    REQUIRE(keys(index.entries()) == vs{"alice.txt", "bob.txt"});
    REQUIRE(index.entries().at("alice.txt").header.modeString() == "100644");
    REQUIRE(index.entries().at("bob.txt").header.modeString() == "100644");
  }
  SECTION("adds a directory") {
    auto alice = createFile(dir / "a-dir/alice.txt", "alice");
    auto bob = createFile(dir / "b-dir/bob.txt", "bob");
    auto args = vs{"a-dir", "b-dir/bob.txt"};
    CHECK(cmd.run(args, {}));
    CHECK(index.readFromFile());
    REQUIRE(keys(index.entries()) == vs{"a-dir/alice.txt", "b-dir/bob.txt"});
    REQUIRE(index.entries().at("a-dir/alice.txt").header.modeString() ==
            "100644");
    REQUIRE(index.entries().at("b-dir/bob.txt").header.modeString() ==
            "100644");
  }
  SECTION("adds root of project") {
    auto alice = createFile(dir / "a-dir/alice.txt", "alice");
    auto bob = createFile(dir / "b-dir/bob.txt", "bob");
    auto args = vs{"."};
    CHECK(cmd.run(args, {}));
    CHECK(index.readFromFile());
    REQUIRE(keys(index.entries()) == vs{"a-dir/alice.txt", "b-dir/bob.txt"});
    REQUIRE(index.entries().at("a-dir/alice.txt").header.modeString() ==
            "100644");
    REQUIRE(index.entries().at("b-dir/bob.txt").header.modeString() ==
            "100644");
  }
  SECTION("fails for non existent files") {
    auto alice = createFile(dir / "alice.txt", "alice");
    auto bob = createFile(dir / "bob.txt", "bob");
    CHECK(index.lock());
    CHECK(index.add(alice));
    CHECK(index.writeToFile());
    auto args = vs{"bob.txt", "nosuchfile.txt"};
    auto res = cmd.run(args, {});
    REQUIRE(isFatal(res.error()));
    CHECK(index.readFromFile());
    REQUIRE(keys(index.entries()) == vs{"alice.txt"});
    REQUIRE(index.entries().at("alice.txt").header.modeString() == "100644");
  }
  SECTION("fails on unreadable files") {
    auto alice = createFile(dir / "alice.txt", "alice");
    auto bob = createFile(dir / "bob.txt", "bob");
    auto charlie = createFile(dir / "charlie.txt", "charlie");
    fs::permissions(charlie,
                    fs::perms::owner_read | fs::perms::group_read |
                        fs::perms::others_read,
                    fs::perm_options::remove);
    CHECK(index.lock());
    CHECK(index.add(alice));
    CHECK(index.writeToFile());
    auto args = vs{"bob.txt", "charlie.txt"};
    auto res = cmd.run(args, {});
    REQUIRE(!res);
    CHECK(index.readFromFile());
    REQUIRE(keys(index.entries()) == vs{"alice.txt"});
    REQUIRE(index.entries().at("alice.txt").header.modeString() == "100644");
  }
}
} // namespace toygit
