#include "toygit/zlib.hpp"
#include <print>

#include <cassert>
#include <stdexcept>
#include <zlib.h>

namespace toygit {
template <typename CharT> Bytef *zptr(CharT *c) {
  return reinterpret_cast<Bytef *>(const_cast<std::remove_cvref_t<CharT> *>(c));
}

std::tuple<ssize_t, ssize_t> DeflateStream::deflateSome(std::string_view in,
                                                        std::span<char> out,
                                                        bool flush) {
  stream_.next_in = zptr(in.data());
  stream_.avail_in = std::ssize(in);
  stream_.next_out = zptr(out.data());
  stream_.avail_out = std::ssize(out);
  auto ret = ::deflate(&stream_, flush ? Z_FINISH : Z_NO_FLUSH);
  assert(ret != Z_STREAM_ERROR);
  auto read = std::ssize(in) - stream_.avail_in;
  auto written = std::ssize(out) - stream_.avail_out;
  return {read, written};
}

std::string DeflateStream::deflate(std::string_view in) {
  auto out = std::string(::deflateBound(&stream_, std::ssize(in)), 0);
  auto [r, w] = deflateSome(in, out, Z_FINISH);
  assert(r == std::ssize(in));
  return out;
}

std::tuple<ssize_t, ssize_t> InflateStream::inflateSome(std::string_view in,
                                                        std::span<char> out,
                                                        bool flush) {
  stream_.next_in = zptr(in.data());
  stream_.avail_in = std::ssize(in);
  stream_.next_out = zptr(out.data());
  stream_.avail_out = std::ssize(out);
  auto ret = ::inflate(&stream_, flush ? Z_FINISH : Z_NO_FLUSH);
  assert(ret != Z_STREAM_ERROR);
  auto read = std::ssize(in) - stream_.avail_in;
  auto written = std::ssize(out) - stream_.avail_out;
  return {read, written};
}

std::string InflateStream::inflate(std::string_view in) {
  static constexpr auto CHUNK = 1024 * 16Z;

  auto out = std::string{};
  auto buf = std::string(CHUNK, 0);
  do {
    buf.resize(CHUNK);
    auto [r, w] = inflateSome(in, buf, Z_FINISH);
    in = in.substr(r);
    buf.resize(w);
    out.append(buf);
  } while (!buf.empty());
  return out;
}

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
