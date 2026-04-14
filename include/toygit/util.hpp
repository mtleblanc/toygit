namespace toygit {
class NoMove {
public:
  NoMove() = default;
  NoMove(const NoMove &) = delete;
  NoMove(NoMove &&) = delete;
  NoMove &operator=(const NoMove &) = delete;
  NoMove &operator==(const NoMove &) = delete;
};
} // namespace toygit
