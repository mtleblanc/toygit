#include "toygit/zlib.hpp"
#include <iostream>

using namespace toygit;

int main() {
  auto content = std::string{std::istreambuf_iterator<char>{std::cin},
                             std::istreambuf_iterator<char>{}};

  auto inflated = InflateStream::inflateOnce(content);
  std::cout << inflated;
  return 0;
}
