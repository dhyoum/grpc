#include <gtest/gtest.h>

#include "util/sum/sum.h"

TEST(SumTest, SumOfOneToHundred) {
    EXPECT_EQ(sum::compute_sum(100), 5050);
}

TEST(SumTest, SumOfOneToOne) {
    EXPECT_EQ(sum::compute_sum(1), 1);
}

TEST(SumTest, SumOfOneToTen) {
    EXPECT_EQ(sum::compute_sum(10), 55);
}

TEST(SumTest, Zero) {
    EXPECT_EQ(sum::compute_sum(0), 0);
}

TEST(SumTest, Negative) {
    EXPECT_EQ(sum::compute_sum(-1), 0);
}
