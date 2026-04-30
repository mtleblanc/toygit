#include "toygit/commit.hpp"
#include "toygit/zlib.hpp"
#include <cassert>
#include <print>

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

  if (args[0] == "commit") {
    doCommit();
    return 0;
  }

  return 0;
}
