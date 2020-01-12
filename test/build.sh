#!/bin/bash

cd /var/app/build || exit

/usr/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ -G "CodeBlocks - Unix Makefiles" /var/app
/usr/bin/cmake --build /var/app/build --target all -- -j 4
