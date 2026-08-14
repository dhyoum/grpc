#include <gtest/gtest.h>

#include "util/factorial/factorial.h"

TEST(FactorialTest, Zero) {
    EXPECT_EQ(factorial::compute(0), 1);
}

TEST(FactorialTest, One) {
    EXPECT_EQ(factorial::compute(1), 1);
}

TEST(FactorialTest, Five) {
    EXPECT_EQ(factorial::compute(5), 120);
}

TEST(FactorialTest, Ten) {
    EXPECT_EQ(factorial::compute(10), 3628800);
}

TEST(FactorialTest, Twenty) {
    EXPECT_EQ(factorial::compute(20), 2432902008176640000LL);
}

TEST(FactorialTest, Negative) {
    EXPECT_EQ(factorial::compute(-1), 0);
    EXPECT_EQ(factorial::compute(-100), 0);
}
