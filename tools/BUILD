package(default_visibility = ["//visibility:public"])

sh_binary(
    name = "clang_tidy_wrapper",
    srcs = ["clang_tidy_wrapper.sh"],
)

# Point to the system clang-tidy binary
genrule(
    name = "clang_tidy_bin_gen",
    outs = ["clang_tidy_bin.sh"],
    cmd = "echo '#!/bin/bash' > $@ && echo 'exec /app/vbuild/RHEL9-x86_64/clang/20.1.1/bin/clang-tidy \"$$@\"' >> $@",
)

sh_binary(
    name = "clang_tidy_bin",
    srcs = ["clang_tidy_bin.sh"],
)
