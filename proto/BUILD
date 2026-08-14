load("@com_github_grpc_grpc//bazel:cc_grpc_library.bzl", "cc_grpc_library")
load("@protobuf//bazel:cc_proto_library.bzl", "cc_proto_library")
load("@protobuf//bazel:proto_library.bzl", "proto_library")

proto_library(
    name = "pingpong_proto",
    srcs = ["pingpong.proto"],
)

cc_proto_library(
    name = "pingpong_cc_proto",
    deps = [":pingpong_proto"],
    visibility = ["//visibility:public"],
)

cc_grpc_library(
    name = "pingpong_cc_grpc",
    srcs = [":pingpong_proto"],
    grpc_only = True,
    deps = [":pingpong_cc_proto"],
    visibility = ["//visibility:public"],
)
