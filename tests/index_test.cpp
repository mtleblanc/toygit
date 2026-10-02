
#include "temp_dir.hpp"
#include "toygit/dircache.hpp"
#include <catch2/catch_test_macros.hpp>
#include <fstream>

namespace toygit {
TEST_CASE("Adding single entry to index", "[index]") {
  auto dir = test::TempDir{};
  std::filesystem::create_directory(dir / std::filesystem::path{".git"});
  auto repository = std::make_shared<Repository>(dir, ".git");
  auto index = DirCache{repository};

  auto file = dir.path() / std::filesystem::path{"alice.txt"};
  std::ofstream{file} << "alice";
  *index.readFromFile();
  *index.add(file);
  *index.writeToFile();
  *index.readFromFile();

  REQUIRE(index.entries().size() == 1);
  REQUIRE(index.entries().at(file.filename().string()).filename ==
          file.filename().string());
}
} // namespace toygit
