#/bin/bash

# Small wrapper to test with different compilers/compile options

set -eu

export SOAKING=1

declare -a options=("" "-fno-exceptions -fno-rtti" "-fno-exceptions -fno-rtti -m32")

COMPILERS=$({ compgen -c g++ & compgen -c clang++; } | sort | uniq)

for compiler in $COMPILERS; do
    for build in Debug Release; do
        for standard in c++11 c++14 c++17 c++2a; do
            for opts in "${options[@]}"; do
                CXX=$compiler CXXFLAGS="-std=$standard $opts" cmake .. -DCMAKE_BUILD_TYPE=$build -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -GNinja
                ninja
                ninja test
                ninja clean
                rm -f CMakeCache.txt
            done
        done
    done
done
