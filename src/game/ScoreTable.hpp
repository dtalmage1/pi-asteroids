#pragma once
#include <array>
#include <cstddef>

namespace ast {

constexpr std::size_t kScoreTableSize = 5;

class ScoreTable {
public:
    void submit(int score) noexcept;

    std::size_t count() const noexcept;
    int         entry(std::size_t index) const noexcept;

private:
    std::array<int, kScoreTableSize> entries_{};
    std::size_t                      count_ = 0U;
};

} // namespace ast
