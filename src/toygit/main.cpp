#include "toygit/core.hpp"
#include "toygit/zlib.hpp"
#include <fstream>

using namespace toygit;

int main() {
  auto ifs = std::ifstream{"./src/toygit/main.cpp"};
  auto inflated = std::string{std::istreambuf_iterator<char>{ifs},
                              std::istreambuf_iterator<char>{}};
  storeBlob(inflated);
  return 0;
}
