#!/bin/bash
set -e
cd "$(dirname "$0")"
mkdir -p build
cd build
{ cmake .. -DCMAKE_BUILD_TYPE=Release 2>&1; echo "---CMAKE_EXIT=$?---"; cmake --build . -j4 2>&1; echo "---BUILD_EXIT=$?---"; } | tee ../build_output.txt
echo "Build finished. See build_output.txt for full log."
