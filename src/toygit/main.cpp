#include "toygit/core.hpp"
#include "toygit/zlib.hpp"

using namespace toygit;

int main() {
  auto d = Tree::buildFrom(std::filesystem::path{"."});
  d->store();
  return 0;
}
