#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$(readlink -f "$0")")"
export ZIG_GLOBAL_CACHE_DIR=$PWD/build/zig-cache ZIG_LOCAL_CACHE_DIR=$PWD/build/zig-cache
mkdir -p build
zig c++ -target x86_64-windows-gnu -std=c++23 -O2 -Wall -Wextra -Wno-nullability-completeness -Iinclude -shared -s \
    -o realistic_gear_ratios.dll src/realistic_gear_ratios.cpp
rm -f realistic_gear_ratios.lib
echo "built $(pwd)/realistic_gear_ratios.dll"
