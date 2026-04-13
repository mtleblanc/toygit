#include "toygit/zlib.hpp"

#include <stdexcept>
#include <string>
#include <vector>
#include <zlib.h>

namespace toygit {

std::vector<char> deflate(const std::vector<char> &input) {
  z_stream s{};
  if (deflateInit(&s, Z_DEFAULT_COMPRESSION) != Z_OK)
    throw std::runtime_error("deflateInit failed");

  std::vector<char> output(deflateBound(&s, input.size()));

  s.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(input.data()));
  s.avail_in = input.size();
  s.next_out = reinterpret_cast<Bytef *>(output.data());
  s.avail_out = output.size();

  int ret = ::deflate(&s, Z_FINISH);
  deflateEnd(&s);

  if (ret != Z_STREAM_END)
    throw std::runtime_error("deflate failed");

  output.resize(s.total_out);
  return output;
}

std::vector<char> inflate(const std::vector<char> &input, size_t max_output) {
  z_stream s{};
  if (inflateInit(&s) != Z_OK)
    throw std::runtime_error("inflateInit failed");

  std::vector<char> output(max_output);

  s.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(input.data()));
  s.avail_in = input.size();
  s.next_out = reinterpret_cast<Bytef *>(output.data());
  s.avail_out = output.size();

  int ret = ::inflate(&s, Z_FINISH);
  inflateEnd(&s);

  if (ret != Z_STREAM_END)
    throw std::runtime_error("inflate failed");

  output.resize(s.total_out);
  return output;
}
} // namespace toygit
