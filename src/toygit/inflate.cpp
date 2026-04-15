#include "toygit/zlib.hpp"
#include <iostream>

int main() {
  auto content = std::string{std::istreambuf_iterator<char>{std::cin},
                             std::istreambuf_iterator<char>{}};

  auto is = toygit::InflateStream{};
  auto inflated = is.inflate(content);
  std::cout << inflated;
  return 0;
}
