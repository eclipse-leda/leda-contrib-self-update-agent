set(PAHO_ENABLE_TESTING FALSE CACHE BOOL "")
set(PAHO_BUILD_STATIC   FALSE CACHE BOOL "")
set(PAHO_BUILD_SHARED   TRUE  CACHE BOOL "")
set(PAHO_BUILD_WITH_SSL FALSE CACHE BOOL "")
add_subdirectory(./paho.mqtt.c)

set(PAHO_MQTT_C_LIBRARIES    ${CMAKE_BINARY_DIR}/3rdparty/paho.mqtt.c/src/libpaho-mqtt3c.so)
set(PAHO_MQTT_C_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/3rdparty/paho.mqtt.c/src)

add_subdirectory(./paho.mqtt.cpp)

add_dependencies(paho-mqttpp3 paho-mqtt3a)
add_dependencies(paho-mqttpp3 paho-mqtt3c)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/lib/pkgconfig)
find_package(OpenSSL REQUIRED)
set(CURL_USE_OPENSSL  ON)
set(BUILD_SHARED_LIBS ON)
set(BUILD_CURL_EXE    OFF)
add_subdirectory(curl)

if(SUA_BUILD_TESTS)
  add_subdirectory(googletest)
endif()

install(
    FILES
        ${CMAKE_BINARY_DIR}/lib/libssl.so.3
        ${CMAKE_BINARY_DIR}/lib/libcrypto.so.3
        ${CMAKE_BINARY_DIR}/3rdparty/curl/lib/libcurl.so
        ${CMAKE_BINARY_DIR}/glib/subprojects/zlib-1.2.11/libz.so
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

install(FILES ${CMAKE_BINARY_DIR}/glib/gio/libgio-2.0.so.0.7800.0             DESTINATION ${CMAKE_INSTALL_PREFIX}/lib RENAME libgio-2.0.so.0)
install(FILES ${CMAKE_BINARY_DIR}/glib/glib/libglib-2.0.so.0.7800.0           DESTINATION ${CMAKE_INSTALL_PREFIX}/lib RENAME libglib-2.0.so.0)
install(FILES ${CMAKE_BINARY_DIR}/glib/gmodule/libgmodule-2.0.so.0.7800.0     DESTINATION ${CMAKE_INSTALL_PREFIX}/lib RENAME libgmodule-2.0.so.0)
install(FILES ${CMAKE_BINARY_DIR}/glib/gobject/libgobject-2.0.so.0.7800.0     DESTINATION ${CMAKE_INSTALL_PREFIX}/lib RENAME libgobject-2.0.so.0)
install(FILES ${CMAKE_BINARY_DIR}/glib/subprojects/libffi/src/libffi.so.7.1.0 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib RENAME libffi.so.7)
install(FILES ${CMAKE_BINARY_DIR}/glib/subprojects/proxy-libintl/libintl.so.8 DESTINATION ${CMAKE_INSTALL_PREFIX}/lib RENAME libintl.so.8)
