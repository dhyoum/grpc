#!/bin/bash
# Wrapper script for clang-tidy that captures output to a file.
# Usage: clang_tidy_wrapper.sh <clang-tidy-binary> <source-file> <output-file> [extra flags...]

CLANG_TIDY="$1"
shift
SOURCE="$1"
shift
OUTPUT="$1"
shift

# GCC 14.2.0 include paths for C++23 standard library headers
GCC_BASE="/app/vbuild/RHEL9-x86_64/gcc/14.2.0"
GCC_INCLUDES=(
    "-isystem" "${GCC_BASE}/include/c++/14.2.0"
    "-isystem" "${GCC_BASE}/include/c++/14.2.0/x86_64-pc-linux-gnu"
    "-isystem" "${GCC_BASE}/include/c++/14.2.0/backward"
    "-isystem" "${GCC_BASE}/lib/gcc/x86_64-pc-linux-gnu/14.2.0/include"
    "-isystem" "${GCC_BASE}/lib/gcc/x86_64-pc-linux-gnu/14.2.0/include-fixed"
)

# Run clang-tidy with GCC headers and C++23
"${CLANG_TIDY}" "${SOURCE}" -- -std=c++23 "${GCC_INCLUDES[@]}" "$@" > "${OUTPUT}" 2>&1 || true

# If output is empty, write a success message
if [ ! -s "${OUTPUT}" ]; then
    echo "clang-tidy: no warnings for ${SOURCE}" > "${OUTPUT}"
fi
