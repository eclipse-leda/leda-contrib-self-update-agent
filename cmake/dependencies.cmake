add_library(curl_lib SHARED IMPORTED)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set_property(
        TARGET curl_lib
        PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/3rdparty/curl/lib/libcurl-d.so
    )
else ()
    set_property(
        TARGET curl_lib
        PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/3rdparty/curl/lib/libcurl.so
    )
endif()

add_library(ssl_lib SHARED IMPORTED)
set_property(
    TARGET ssl_lib
    PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libssl.so.3
)

add_library(crypto_lib SHARED IMPORTED)
set_property(
    TARGET crypto_lib
    PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/lib/libcrypto.so.3
)

add_library(gio_lib SHARED IMPORTED)
set_property(
    TARGET gio_lib
    PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/glib/gio/libgio-2.0.so
)

add_library(glib_lib SHARED IMPORTED)
set_property(
    TARGET glib_lib
    PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/glib/glib/libglib-2.0.so
)

add_library(gmodule_lib SHARED IMPORTED)
set_property(
    TARGET gmodule_lib
    PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/glib/gmodule/libgmodule-2.0.so
)

add_library(gobject_lib SHARED IMPORTED)
set_property(
    TARGET gobject_lib
    PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/glib/gobject/libgobject-2.0.so
)

add_library(ffi_lib SHARED IMPORTED)
set_property(
    TARGET ffi_lib
    PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/glib/subprojects/libffi/src/libffi.so
)

add_library(z_lib SHARED IMPORTED)
set_property(
    TARGET z_lib
    PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/glib/subprojects/zlib-1.2.11/libz.so
)

add_library(intl_lib SHARED IMPORTED)
set_property(
    TARGET intl_lib
    PROPERTY IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/glib/subprojects/proxy-libintl/libintl.so
)
