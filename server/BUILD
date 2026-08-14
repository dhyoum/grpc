cc_binary(
    name = "server",
    srcs = ["server.cc"],
    deps = [
        "//proto:pingpong_cc_grpc",
        "//proto:pingpong_cc_proto",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_test(
    name = "pingpong_test",
    srcs = ["pingpong_test.cc"],
    deps = [
        "//proto:pingpong_cc_grpc",
        "//proto:pingpong_cc_proto",
        "@com_github_grpc_grpc//:grpc++",
        "@googletest//:gtest_main",
    ],
)
