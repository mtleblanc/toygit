#include "toygit/core.hpp"
#include "toygit/zlib.hpp"
#include <cassert>
#include <fstream>

using namespace toygit;

int main() {
  auto d = Tree::buildFrom(std::filesystem::path{"."});
  d->store();
  auto is = std::ifstream{".git/HEAD"};
  std::string ref;
  std::string headPath;
  is >> ref >> headPath;
  assert(ref == "ref:");
  auto headRefPath = std::filesystem::path(".git");
  headRefPath.append(headPath);
  std::optional<std::string> parent{};
  if (std::filesystem::status(headRefPath).type() !=
      std::filesystem::file_type::not_found) {
    parent = std::string{};
    auto ifs = std::ifstream{headRefPath};
    ifs >> *parent;
  }

  auto commit = Commit{d->id(), parent, "Michael LeBlanc",
                       std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count(),
                       "Message"};
  commit.store();

  auto ofs = std::ofstream{headRefPath};
  auto id = commit.id();
  ofs << hexString(id);
  return 0;
}
