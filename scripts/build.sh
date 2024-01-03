#!/bin/sh

if [ "$1" = "amd64" ]; then
  ./scripts/build_openssl_amd64.sh
  ./scripts/build_glib_amd64.sh
  cd build_amd64
  cmake -DCMAKE_INSTALL_PREFIX=../dist_amd64 -DCMAKE_TOOLCHAIN_FILE=../cmake/linux/amd64/toolchain.cmake -DOPENSSL_ROOT_DIR=../build_amd64 -DOPENSSL_CRYPTO_LIBRARY=../build_amd64/lib/libcrypto.so ..
  make install
exit 0
fi

if [ "$1" = "arm64" ]; then
  ./scripts/build_openssl_arm64.sh
  ./scripts/build_glib_arm64.sh
  cd build_arm64
  cmake -DCMAKE_INSTALL_PREFIX=../dist_arm64 -DCMAKE_TOOLCHAIN_FILE=../cmake/linux/arm64/toolchain.cmake -DOPENSSL_ROOT_DIR=../build_arm64 -DOPENSSL_CRYPTO_LIBRARY=../build_arm64/lib/libcrypto.so ..
  make install
  exit 0
fi

echo "Unknown architecture '$1'"

