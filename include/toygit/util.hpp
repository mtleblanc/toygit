#include <expected>
#include <system_error>
namespace toygit {

template <typename T> using Result = std::expected<T, std::error_code>;

class NoMove {
public:
  NoMove() = default;
  NoMove(const NoMove &) = delete;
  NoMove(NoMove &&) = delete;
  NoMove &operator=(const NoMove &) = delete;
  NoMove &operator=(NoMove &&) = delete;
};

class MoveOnly {
public:
  MoveOnly() = default;
  MoveOnly(const MoveOnly &) = delete;
  MoveOnly(MoveOnly &&) = default;
  MoveOnly &operator=(const MoveOnly &) = delete;
  MoveOnly &operator=(MoveOnly &&) = default;
};
} // namespace toygit
