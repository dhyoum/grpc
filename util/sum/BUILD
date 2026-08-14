cc_library(
    name = "sum",
    srcs = ["sum.cc"],
    hdrs = ["sum.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "sum_test",
    srcs = ["sum_test.cc"],
    deps = [
        ":sum",
        "@googletest//:gtest_main",
    ],
)
