#include "toygit/core.hpp"
#include "toygit/util.hpp"
#include <cstdint>
#include <string>
#include <sys/stat.h>
#include <utility>

namespace toygit {

class DirCache {
public:
  static DirCache readFromFile();
  Result<void> writeToFile();
  Result<void> add(const std::filesystem::path &file);
  void listFiles();

  struct EntryHeader {
    uint32_t ctimeSeconds;
    uint32_t ctimeNanos;
    uint32_t mtimeSeconds;
    uint32_t mtimeNanos;
    uint32_t device;
    uint32_t inode;
    uint32_t mode;
    uint32_t uid;
    uint32_t gid;
    uint32_t size;
    Id id;
    uint16_t flags;

    EntryHeader &swapEndian();
    std::string_view modeString() const {
      static constinit auto DIR = "40000";
      static constinit auto REGULAR = "100644";
      static constinit auto EXECUTABLE = "100755";
      static constinit auto SYMLINK = "120000";
      switch (mode) {
      case 040000:
        return std::string_view{DIR};
      case 0100644:
        return std::string_view{REGULAR};
      case 0100755:
        return std::string_view{EXECUTABLE};
      case 0120000:
        return std::string_view{SYMLINK};
      default:
        std::unreachable();
      }
    }

    static uint32_t restrictMode(uint32_t mode) {
      auto fileType = mode & S_IFMT;
      if (fileType == S_IFLNK) {
        return 0120000;
      }
      if (fileType != S_IFREG) {
        throw std::runtime_error{"Adding non supported file"};
      }
      return (mode & S_IXUSR) != 0 ? 0100755 : 0100644;
    }

    static EntryHeader fromStat(const struct stat &stat) {
      return {.ctimeSeconds = static_cast<uint32_t>(stat.st_ctim.tv_sec),
              .ctimeNanos = static_cast<uint32_t>(stat.st_ctim.tv_nsec),
              .mtimeSeconds = static_cast<uint32_t>(stat.st_mtim.tv_sec),
              .mtimeNanos = static_cast<uint32_t>(stat.st_mtim.tv_nsec),
              .device = static_cast<uint32_t>(stat.st_dev),
              .inode = static_cast<uint32_t>(stat.st_ino),
              .mode = restrictMode(static_cast<uint32_t>(stat.st_mode)),
              .uid = static_cast<uint32_t>(stat.st_uid),
              .gid = static_cast<uint32_t>(stat.st_gid),
              .size = static_cast<uint32_t>(stat.st_size),
              .id = {},
              .flags = 0};
    }
  };

  struct Entry {
    EntryHeader header;
    std::string filename;
  };

private:
  int32_t version{};
  std::map<std::string, Entry> entries{};
};
} // namespace toygit
