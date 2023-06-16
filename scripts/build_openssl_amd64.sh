#!/bin/sh

rootdir=`pwd`
mkdir -p build_amd64/3rdparty/openssl
cd build_amd64/3rdparty/openssl
./../../../3rdparty/openssl/Configure \
	--prefix=$rootdir/build_amd64 \
	--openssldir=$rootdir/build_amd64 \
	--libdir=lib \
	shared \
	-Wl,-rpath=$rootdir/build_amd64/lib \
	-Wl,--enable-new-dtags > /dev/null
make -j > /dev/null
make install_sw > /dev/null
