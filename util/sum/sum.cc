#include "util/sum/sum.h"

#include <algorithm>
#include <functional>
#include <ranges>

namespace sum {

int compute_sum(int n) {
    if (n <= 0) return 0;
    // C++23 ranges + iota를 활용한 합산
    auto numbers = std::views::iota(1, n + 1);
    return std::ranges::fold_left(numbers, 0, std::plus<>{});
}

}  // namespace sum
