cc_library(
    name = "factorial",
    srcs = ["factorial.cc"],
    hdrs = ["factorial.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "factorial_test",
    srcs = ["factorial_test.cc"],
    deps = [
        ":factorial",
        "@googletest//:gtest_main",
    ],
)
