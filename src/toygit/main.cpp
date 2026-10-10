#include "toygit/add_command.hpp"
#include "toygit/commit.hpp"
#include "toygit/core.hpp"
#include "toygit/dircache.hpp"
#include "toygit/error.hpp"
#include "toygit/status_command.hpp"
#include <cassert>
#include <filesystem>
#include <iostream>
#include <print>
#include <vector>

using namespace toygit;

void printUsage(std::string_view programName) {
  std::println("Usage: {} <command> <command-args...>", programName);
  std::println("Commands:");
  std::println("  init");
  std::println("  commit");
  std::println("  add");
}

int main(int argc, char *argv[]) {
  std::string programName{argv[0]};
  if (argc < 2) {
    printUsage(programName);
    return 0;
  }
  auto command = std::string{*(argv + 1)};
  std::vector<std::string> args(argv + 2, argv + argc);

  auto repo = std::make_shared<Repository>(std::filesystem::current_path(),
                                           std::filesystem::path(".toygit"));

  auto comm = std::unique_ptr<Command>{};
  if (command == "add") {
    comm = std::make_unique<AddCommand>(repo);
  } else if (command == "status") {
    comm = std::make_unique<StatusCommand>(repo);
  }

  if (comm) {
    auto res = comm->run(args, {});
    if (res) {
      return 0;
    }
    auto err = res.error();
    // assume if we have a GitError that diagnostics were already printed.
    if (!isGitError(err)) {
      std::println(std::cerr, "{}", res.error().message());
    }
    return res.error().value();
  }

  try {
    if (command == "commit") {
      doCommit();
      return 0;
    }

    if (command == "list-files") {
      auto index = DirCache{repo};
      std::ignore = index.readFromFile();
      index.listFiles();
    }

    if (command == "tree") {
      auto tree = Tree::buildFromIndex(repo);
      auto digest = tree->id();
      std::println("{}", hexString(std::span{digest}));
    }
  } catch (const std::exception &e) {
    std::println("Error: {}", e.what());
    return 1;
  }

  return 0;
}
