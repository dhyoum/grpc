#include "util/factorial/factorial.h"

#include <algorithm>
#include <functional>
#include <ranges>

namespace factorial {

std::int64_t compute(int n) {
    if (n < 0) return 0;
    if (n == 0) return 1;

    // C++23 ranges::fold_left로 1*2*...*n 계산
    auto numbers = std::views::iota(1, n + 1);
    return std::ranges::fold_left(numbers, std::int64_t{1}, std::multiplies<>{});
}

}  // namespace factorial
