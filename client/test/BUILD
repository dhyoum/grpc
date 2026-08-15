cc_test(
    name = "pingpong_client_test",
    srcs = ["pingpong_client_test.cc"],
    deps = [
        "//client:pingpong_client",
        "//server:pingpong_service",
        "//proto:pingpong_cc_grpc",
        "//proto:pingpong_cc_proto",
        "@com_github_grpc_grpc//:grpc++",
        "@googletest//:gtest_main",
    ],
)
