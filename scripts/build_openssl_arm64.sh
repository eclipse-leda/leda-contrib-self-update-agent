#!/bin/sh

rootdir=`pwd`
mkdir -p build_arm64/3rdparty/openssl
cd build_arm64/3rdparty/openssl
CC=aarch64-linux-gnu-gcc-9 \
CXX=aarch64-linux-gnu-g++-9 \
LD=aarch64-linux-gnu-ld \
AR=aarch64-linux-gnu-ar \
RANLIB=aarch64-linux-gnu-ranlib \
    ./../../../3rdparty/openssl/Configure linux-aarch64 \
        --prefix=$rootdir/build_arm64 \
        --openssldir=$rootdir/build_arm64 \
        --libdir=lib \
        shared \
        -Wl,-rpath=$rootdir/build_arm64/lib \
        -Wl,--enable-new-dtags > /dev/null
make -j > /dev/null
make install_sw > /dev/null
