#!/bin/sh

cd 3rdparty/glib
meson subprojects download
meson setup -Dwrap_mode=forcefallback --cross-file=../meson-cross-file-aarch64.txt ../../build_arm64/glib
meson compile -j `nproc` -C ../../build_arm64/glib
