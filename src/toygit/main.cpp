#include "toygit/zlib.hpp"
#include <iostream>
#include <vector>

int main() {
  std::vector<char> content{std::istreambuf_iterator<char>{std::cin},
                            std::istreambuf_iterator<char>{}};
  std::vector<char> inflated = toygit::inflate(content, content.size() * 16);
  std::string str{inflated.begin(), inflated.end()};
  std::cout << str;
  return 0;
}
