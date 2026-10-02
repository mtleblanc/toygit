
#include "temp_dir.hpp"
#include "toygit/dircache.hpp"
#include <catch2/catch_test_macros.hpp>
#include <fstream>

namespace toygit {

namespace fs = std::filesystem;
using namespace std::string_literals;
fs::path operator"" _p(const char *p, [[maybe_unused]] size_t len) {
  return fs::path{p};
}

TEST_CASE("Adding single entry to index", "[index]") {
  auto dir = test::TempDir{};
  fs::create_directory(dir / ".git"_p);
  auto repository = std::make_shared<Repository>(dir, ".git");
  auto index = DirCache{repository};
  SECTION("adding single file") {
    auto file = dir / "alice.txt"_p;
    std::ofstream{file} << "alice";
    *index.add(file);

    CHECK(index.entries().size() == 1);
    REQUIRE(index.entries().at(file.filename().string()).filename ==
            file.filename().string());
  }

  SECTION("replaces a file with a directory") {
    auto aliceFile = dir / "alice.txt"_p;
    auto bobFile = dir / "bob.txt"_p;
    std::ofstream{aliceFile} << "alice";
    std::ofstream{bobFile} << "bob";
    *index.add(aliceFile);
    *index.add(bobFile);
    std::filesystem::remove(aliceFile);
    std::filesystem::create_directory(aliceFile);
    auto fileInAliceDir = aliceFile / "sub_alice.txt";
    std::ofstream{fileInAliceDir} << "alice sub";
    *index.add(fileInAliceDir);
    CHECK(index.entries().size() == 2);
    REQUIRE(index.entries().contains("bob.txt"s));
    REQUIRE(index.entries().contains("alice.txt/sub_alice.txt"));
  }
}
} // namespace toygit
