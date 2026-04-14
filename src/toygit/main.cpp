#include "toygit/zlib.hpp"
#include <fstream>
#include <iostream>

int main() {
  auto ifs =
      std::ifstream{".git/objects/b3/a163ebd0a3290dad353cafe1fd8e5dd4e08aa4"};
  auto content = std::string{std::istreambuf_iterator<char>{ifs},
                             std::istreambuf_iterator<char>{}};
  auto is = toygit::InflateStream{};
  auto inflated = is.inflate(content);
  std::cout << inflated;
  return 0;
}
