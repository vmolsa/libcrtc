#!/bin/sh

CXX="clang++"
CFLAGS="-std=c++11 -fno-exceptions -Idist/include/"
LDFLAGS=" -Ldist/lib/ -lcrtc"

$CXX $CFLAGS $LDFLAGS -o out/mesh examples/mesh.cc