#include <vector>
namespace toygit {
std::vector<char> deflate(const std::vector<char> &input);
std::vector<char> inflate(const std::vector<char> &input, size_t max_output);
} // namespace toygit
