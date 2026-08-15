cc_library(
    name = "pingpong_client",
    srcs = ["pingpong_client.cc"],
    hdrs = ["pingpong_client.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//proto:pingpong_cc_grpc",
        "//proto:pingpong_cc_proto",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "client",
    srcs = ["client.cc"],
    deps = [
        ":pingpong_client",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
