#pragma once

#include "toygit/file.hpp"
#include <fcntl.h>
#include <filesystem>
#include <unistd.h>

namespace toygit {

class Lockfile {
public:
  Lockfile(std::filesystem::path file);
  ~Lockfile();

  Result<void> write(std::string_view sv);
  Result<void> commit();
  Result<void> release();

private:
  enum class State { Open, Committed, Released };

  State state_ = State::Open;
  std::filesystem::path file_;
  std::filesystem::path lockFile_;
  File lf_;
};
} // namespace toygit
