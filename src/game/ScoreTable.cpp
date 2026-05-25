#include "game/ScoreTable.hpp"
#include <algorithm>

namespace ast {

void ScoreTable::submit(int score) noexcept {
    if (count_ < kScoreTableSize) {
        entries_.at(count_) = score;
        ++count_;
    } else if (score > entries_.at(count_ - 1U)) {
        entries_.at(count_ - 1U) = score;
    } else {
        return;
    }
    std::sort(entries_.begin(),
              entries_.begin() + static_cast<std::ptrdiff_t>(count_),
              [](int a, int b) { return a > b; });
}

std::size_t ScoreTable::count() const noexcept { return count_; }

int ScoreTable::entry(std::size_t index) const noexcept {
    return entries_.at(index);
}

} // namespace ast
