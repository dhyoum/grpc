#include <print>

#include "util/sum/sum.h"

int main() {
    int result = sum::compute_sum(100);
    std::println("1부터 100까지의 합: {}", result);
    return 0;
}
