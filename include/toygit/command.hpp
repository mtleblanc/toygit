#pragma once

#include "toygit/repository.hpp"
#include "toygit/util.hpp"
#include <map>
#include <memory>

namespace toygit {

class Command {
public:
  Command(std::shared_ptr<Repository> repository) : repository{repository} {}

  virtual Result<void> run(std::span<std::string> args,
                           const std::map<std::string, std::string> &env) = 0;

  virtual ~Command() = default;

protected:
  std::shared_ptr<Repository> repository;
};
} // namespace toygit
