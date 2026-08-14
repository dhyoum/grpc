"""Clang-tidy aspect for Bazel C++ targets."""

ClangTidyInfo = provider(
    doc = "Provider for clang-tidy output files.",
    fields = {"output": "The clang-tidy report file."},
)

def _clang_tidy_aspect_impl(target, ctx):
    if CcInfo not in target:
        return []

    # Collect all C/C++ source files
    srcs = []
    if hasattr(ctx.rule.attr, "srcs"):
        for src in ctx.rule.attr.srcs:
            for f in src.files.to_list():
                if f.extension in ["cc", "cpp", "cxx", "c"]:
                    srcs.append(f)

    if not srcs:
        return []

    # Collect headers for include paths
    compilation_context = target[CcInfo].compilation_context
    headers = compilation_context.headers.to_list()

    # Build include flags
    include_flags = []
    for dir in compilation_context.includes.to_list():
        include_flags.extend(["-I", dir])
    for dir in compilation_context.system_includes.to_list():
        include_flags.extend(["-isystem", dir])
    for dir in compilation_context.quote_includes.to_list():
        include_flags.extend(["-iquote", dir])

    # Add external include paths
    include_flags.extend(["-isystem", "external"])

    outputs = []
    for src in srcs:
        out = ctx.actions.declare_file(
            "{}.clang-tidy.txt".format(src.short_path),
        )
        outputs.append(out)

        args = ctx.actions.args()
        args.add(ctx.executable._clang_tidy)
        args.add(src)
        args.add(out)
        args.add_all(include_flags)

        ctx.actions.run(
            executable = ctx.executable._clang_tidy_wrapper,
            arguments = [args],
            inputs = depset([src], transitive = [compilation_context.headers]),
            outputs = [out],
            tools = [ctx.executable._clang_tidy],
            mnemonic = "ClangTidy",
            progress_message = "Running clang-tidy on %s" % src.short_path,
        )

    return [
        OutputGroupInfo(clang_tidy = depset(outputs)),
        ClangTidyInfo(output = depset(outputs)),
    ]

clang_tidy_aspect = aspect(
    implementation = _clang_tidy_aspect_impl,
    attr_aspects = ["deps"],
    attrs = {
        "_clang_tidy": attr.label(
            default = "//tools:clang_tidy_bin",
            executable = True,
            cfg = "exec",
            allow_files = True,
        ),
        "_clang_tidy_wrapper": attr.label(
            default = "//tools:clang_tidy_wrapper",
            executable = True,
            cfg = "exec",
            allow_files = True,
        ),
    },
)
