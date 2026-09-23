#include <cstdint>
#include <string>
#include <vector>

namespace toygit {

class DirCache {
public:
  static DirCache readFromFile();

private:
  class Entry {
    int32_t ctimeSeconds;
    int32_t ctimeNanos;
    int32_t mtimeSeconds;
    int32_t mtimeNanos;
    int32_t device;
    int32_t inode;
    int32_t mdoe;
    int32_t uid;
    int32_t gid;
    int32_t size;
    std::string filename;
  };

  int32_t version{};
  std::vector<Entry> entries{};
};
} // namespace toygit
