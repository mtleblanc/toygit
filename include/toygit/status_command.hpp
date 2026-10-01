#pragma once

#include "toygit/command.hpp"

namespace toygit {
class StatusCommand : public Command {
public:
  StatusCommand(std::shared_ptr<Repository> repository) : Command{repository} {}

  Result<void>
  run(std::span<std::string> args,
      [[maybe_unused]] const std::map<std::string, std::string> &env) override;

private:
};
} // namespace toygit
