#include "toygit/util.hpp"
#include <span>
#include <stdexcept>
#include <vector>
#include <zlib.h>

namespace toygit {
std::vector<char> deflate(const std::vector<char> &input);
std::vector<char> inflate(const std::vector<char> &input, size_t max_output);

class DeflateStream : NoMove {
public:
  DeflateStream() {
    if (deflateInit(&stream_, Z_DEFAULT_COMPRESSION) != Z_OK)
      throw std::runtime_error("deflateInit failed");
  }
  std::tuple<ssize_t, ssize_t> deflateSome(std::string_view in,
                                           std::span<char> out, bool flush);
  std::string deflate(std::string_view in);

  ~DeflateStream() { deflateEnd(&stream_); }

private:
  z_stream stream_{};
};

class InflateStream : NoMove {
public:
  InflateStream() {
    if (inflateInit(&stream_) != Z_OK)
      throw std::runtime_error("inflateInit failed");
  }
  std::tuple<ssize_t, ssize_t> inflateSome(std::string_view in,
                                           std::span<char> out, bool flush);
  std::string inflate(std::string_view in);
  ~InflateStream() { inflateEnd(&stream_); }

private:
  z_stream stream_{};
};
} // namespace toygit
