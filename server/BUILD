cc_library(
    name = "pingpong_service",
    srcs = ["pingpong_service.cc"],
    hdrs = ["pingpong_service.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//proto:pingpong_cc_grpc",
        "//proto:pingpong_cc_proto",
        "@com_github_grpc_grpc//:grpc++",
    ],
)

cc_binary(
    name = "server",
    srcs = ["server.cc"],
    deps = [
        ":pingpong_service",
        "@com_github_grpc_grpc//:grpc++",
    ],
)
