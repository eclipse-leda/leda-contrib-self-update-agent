#!/bin/sh

cd 3rdparty/glib
meson subprojects download > /dev/null
meson setup -Dwrap_mode=forcefallback --cross-file=../meson-cross-file-aarch64.txt ../../build_arm64/glib > /dev/null
meson compile -j `nproc` -C ../../build_arm64/glib > /dev/null
