#!/bin/sh

cd 3rdparty/glib
meson subprojects download > /dev/null
meson setup -Dwrap_mode=forcefallback ../../build_amd64/glib > /dev/null
meson compile -j `nproc` -C ../../build_amd64/glib > /dev/null
