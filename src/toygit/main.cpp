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
  std::vector<std::string> args(argv + 1, argv + argc);
  if (args.empty()) {
    printUsage(programName);
    return 0;
  }

  auto repo = std::make_shared<Repository>(std::filesystem::path{""},
                                           std::filesystem::path(".toygit"));
  try {
    if (args[0] == "commit") {
      doCommit();
      return 0;
    }

    if (args[0] == "add") {
      auto index = DirCache{repo};
      std::ignore = index.readFromFile();
      if (args.size() < 2) {
        std::println("Must provide path to file to add");
        return 1;
      }
      std::ignore = index.add(std::filesystem::path(args[1]));
      std::ignore = index.writeToFile();
    }

    if (args[0] == "tree") {
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
