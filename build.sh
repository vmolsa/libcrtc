#!/bin/sh

set -e

export ROOT=$(pwd)
export WEBRTC_COMMIT=ec3619b6b6920ab2d779d0f78cbc9f0d6d88dca7
export WEBRTC_REPOSITORY=https://chromium.googlesource.com/external/webrtc.git@$WEBRTC_COMMIT

if [ ! -d depot_tools ]; then
  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
fi

export PATH=$PATH:$(pwd)/depot_tools

if [ ! -d webrtc ]; then
  mkdir $ROOT/webrtc
fi

if [ ! -f $ROOT/webrtc/.webrtc_sync ]; then
  cd $ROOT/webrtc

  if [ -d $ROOT/webrtc/src ]; then
    git stash
  fi

  gclient config --name=src $WEBRTC_REPOSITORY
  gclient sync -j16 --revision=$WEBRTC_COMMIT --no-history
  ln -fn $ROOT/root.gn $ROOT/webrtc/src/BUILD.gn
  touch .webrtc_sync
  cd $ROOT
fi

if [ ! -d $ROOT/webrtc/src/crtc ]; then
  ln -s $ROOT $ROOT/webrtc/src/crtc
fi

if [ ! -d $ROOT/out ] || [ ! -f $ROOT/out/build.ninja ]; then
  cd $ROOT/webrtc/src
  gn gen ../../out/ --args='is_debug=true is_component_build=true'
  cd $ROOT
fi

ninja -vC out crtc

if [ "$BUILD_TESTS" == "true" ]; then
  ninja -C out test-promise
fi
