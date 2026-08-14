#ifndef UTIL_FACTORIAL_FACTORIAL_H_
#define UTIL_FACTORIAL_FACTORIAL_H_

#include <cstdint>

namespace factorial {

// n! 을 반환합니다. n < 0 이면 0을 반환합니다.
std::int64_t compute(int n);

}  // namespace factorial

#endif  // UTIL_FACTORIAL_FACTORIAL_H_
