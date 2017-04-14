#!/bin/sh

clang++ -Idist/include/ -std=c++11 -stdlib=libc++ -Ldist/lib/ -lcrtc -Wl,-rpath,dist/lib/ -o out/main-example examples/main.cc