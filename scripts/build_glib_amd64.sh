#!/bin/sh

cd 3rdparty/glib
meson subprojects download
meson setup -Dwrap_mode=forcefallback ../../build_amd64/glib
meson compile -j `nproc` -C ../../build_amd64/glib
