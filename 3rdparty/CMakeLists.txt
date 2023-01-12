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

add_library(mini-yaml SHARED ./mini-yaml/yaml/Yaml.cpp)
install(TARGETS mini-yaml LIBRARY DESTINATION lib)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/lib/pkgconfig)
find_package(OpenSSL REQUIRED)
set(CURL_USE_OPENSSL  ON)
set(BUILD_SHARED_LIBS ON)
set(BUILD_CURL_EXE    OFF)
add_subdirectory(curl)

if(SUA_BUILD_TESTS)
  add_subdirectory(googletest)
endif()
