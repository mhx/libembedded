#/bin/bash

# Small wrapper to test with different compilers/compile options

set -eu

export SOAKING=1

declare -a options=("" "-fno-exceptions -fno-rtti" "-fno-exceptions -fno-rtti -m32")

for compiler in g++ g++-10 clang++ clang++-11; do
    for standard in c++11 c++14 c++17 c++2a; do
        for opts in "${options[@]}"; do
            CXX=$compiler CXXFLAGS="-std=$standard $opts" cmake .. -GNinja
            ninja && ninja test && ninja clean && rm -f CMakeCache.txt
        done
    done
done
