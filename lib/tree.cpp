#include "toygit/tree.hpp"
#include "detail/object.hpp"
#include "toygit/core.hpp"
#include "toygit/dircache.hpp"
#include <cassert>
#include <print>
#include <utility>
#include <vector>

namespace toygit {

namespace {
const std::string &modeString(Tree::Mode m) {
  static auto DIR = std::string{"40000"};
  static auto REGULAR = std::string{"100644"};
  static auto EXECUTABLE = std::string{"100755"};
  static auto SYMLINK = std::string{"120000"};
  switch (m) {
  case Tree::Mode::DIRECTORY:
    return DIR;
  case Tree::Mode::REGUALAR_FILE:
    return REGULAR;
  case Tree::Mode::EXECUTABLE_FILE:
    return EXECUTABLE;
  case Tree::Mode::SYMLINK:
    return SYMLINK;
  default:
    std::unreachable();
  }
}

Tree::Mode modeFromIndex(uint32_t mode) {
  switch (mode) {
  case 040000:
    return Tree ::Mode::DIRECTORY;
  case 0100644:
    return Tree::Mode::REGUALAR_FILE;
  case 0100755:
    return Tree::Mode::EXECUTABLE_FILE;
  case 0120000:
    return Tree::Mode::SYMLINK;
  default:
    std::unreachable();
  }
}
} // namespace

std::string_view Tree::content() {
  if (!content_.empty()) {
    return content_;
  }
  std::string text{};
  for (const auto &[name, idMode] : children_) {
    auto &[id, mode] = idMode;
    auto nameView = std::string_view{name};
    if (nameView.ends_with('/')) {
      nameView.remove_suffix(1);
    }
    text.append(modeString(mode));
    text.append(" ");
    text.append(nameView);
    text.append(1, 0);
    text.append(id.begin(), id.end());
  }
  return content_ = packageContent("tree", text);
}

bool shouldIgnore(const std::filesystem::directory_entry &de) {
  if (de.is_regular_file()) {
    return false;
  }
  if (de.is_directory()) {
    auto filename = de.path().filename();
    if (filename == "build" || filename == ".cache" || filename == ".git" ||
        filename == ".toygit") {
      return true;
    }
    return false;
  }
  return true;
}

std::shared_ptr<Tree>
Tree::buildFromIndex(std::shared_ptr<Repository> repository) {
  auto index = DirCache{repository};
  std::ignore = index.readFromFile();

  auto treeStack =
      std::vector<std::tuple<std::string, std::shared_ptr<Tree>>>{};
  treeStack.emplace_back(std::string{}, std::make_shared<Tree>());
  auto currentTree = std::get<1>(treeStack.back());
  auto prefix = std::string{};
  for (auto &[name, data] : index.entries_) {
    while (!name.starts_with(prefix)) {
      assert(!treeStack.empty());
      auto dirName = std::get<0>(treeStack.back());
      treeStack.pop_back();
      currentTree->store();
      auto id = currentTree->id();
      currentTree = std::get<1>(treeStack.back());
      currentTree->children_.emplace(dirName,
                                     std::tuple{id, Tree::Mode::DIRECTORY});
      assert(prefix.ends_with(dirName + "/"));
      prefix.erase(prefix.length() - dirName.length() - 1);
      assert(prefix.empty() || prefix.ends_with('/'));
    }
    auto tail = std::string_view(name).substr(prefix.length());
    while (true)
      if (auto idx = tail.find('/'); idx != std::string_view::npos) {
        treeStack.emplace_back(std::string{tail.substr(0, idx)},
                               std::make_shared<Tree>());
        currentTree = std::get<1>(treeStack.back());
        prefix.append(tail.substr(0, idx + 1));
        tail.remove_prefix(idx + 1);
      } else {
        break;
      }
    assert(!tail.empty());
    currentTree->children_.emplace(
        tail, std::tuple{data.header.id, modeFromIndex(data.header.mode)});
  }

  while (treeStack.size() > 1) {
    auto dirName = std::get<0>(treeStack.back());
    treeStack.pop_back();
    currentTree->store();
    auto id = currentTree->id();
    currentTree = std::get<1>(treeStack.back());
    currentTree->children_.emplace(dirName,
                                   std::tuple{id, Tree::Mode::DIRECTORY});
  }
  currentTree->store();

  return currentTree;
}
std::shared_ptr<Tree> Tree::buildFrom(std::filesystem::path path) {
  namespace fs = std::filesystem;
  auto ec = std::error_code{};
  auto status = fs::status(path, ec);
  if (status.type() != fs::file_type::directory) {
    return {};
  }
  auto res = std::make_shared<Tree>();
  for (auto &de : fs::directory_iterator(path)) {
    if (shouldIgnore(de)) {
      continue;
    }
    auto thisPath = de.path();
    if (de.is_directory()) {
      auto entry = buildFrom(thisPath);
      if (entry) {
        entry->store();
        res->children_[thisPath.filename().string() + "/"] =
            std::make_tuple(entry->id(), Tree::Mode::DIRECTORY);
      }
    }
    if (de.is_symlink()) {
      auto entry = Blob::buildFromSymlink(thisPath);
      if (entry) {
        entry->store();
        res->children_[thisPath.filename()] =
            std::make_tuple(entry->id(), Tree::Mode::SYMLINK);
      }
    } else if (de.is_regular_file()) {
      auto entry = Blob::buildFrom(thisPath);
      if (entry) {
        entry->store();
        auto isExecutable = (de.status().permissions() &
                             fs::perms::owner_exec) != fs::perms::none;
        res->children_[thisPath.filename()] = std::make_tuple(
            entry->id(), isExecutable ? Tree::Mode::EXECUTABLE_FILE
                                      : Tree::Mode::REGUALAR_FILE);
      }
    }
  }
  return res;
}

} // namespace toygit
