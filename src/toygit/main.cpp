#include "toygit/add_command.hpp"
#include "toygit/commit.hpp"
#include "toygit/core.hpp"
#include "toygit/dircache.hpp"
#include <cassert>
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

  auto repo = std::make_shared<Repository>(std::filesystem::path{""},
                                           std::filesystem::path(".toygit"));
  try {
    if (command == "commit") {
      doCommit();
      return 0;
    }

    if (command == "add") {
      auto cmd = AddCommand{repo};
      if (auto res = cmd.run(args, {}); !res) {
        throw res.error();
      };
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
