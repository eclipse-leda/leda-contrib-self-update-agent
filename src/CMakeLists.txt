if(NOT DEFINED SUA_COMMIT_HASH)
    execute_process(
        COMMAND git describe --always --abbrev=40 --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} OUTPUT_VARIABLE SUA_COMMIT_HASH
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

if(NOT DEFINED SUA_BUILD_NUMBER)
    set(SUA_BUILD_NUMBER "local")
endif()

if(SUA_MEASURE_CODE_COVERAGE)
    add_compile_options(
        -fprofile-arcs
        -ftest-coverage
        -fPIC
    )
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/version.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/version.cpp @ONLY)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/3rdparty/spdlog/include
    ${CMAKE_SOURCE_DIR}/3rdparty/paho.mqtt.c/src
    ${CMAKE_SOURCE_DIR}/3rdparty/paho.mqtt.cpp/src
    ${CMAKE_SOURCE_DIR}/3rdparty/glib
    ${CMAKE_SOURCE_DIR}/3rdparty/glib/glib
    ${CMAKE_SOURCE_DIR}/3rdparty/glib/gmodule
    ${CMAKE_SOURCE_DIR}/3rdparty/glib/builddir/
    ${CMAKE_SOURCE_DIR}/3rdparty/nlohmann-json/include
    ${CMAKE_SOURCE_DIR}/3rdparty/curl/include
    ${CMAKE_BINARY_DIR}/3rdparty/curl/lib
    ${CMAKE_BINARY_DIR}/glib
    ${CMAKE_BINARY_DIR}/glib/glib
)

file(GLOB_RECURSE SRCS *.cpp *.h)
list(FILTER SRCS EXCLUDE REGEX "main.cpp" )
list(APPEND SRCS ${CMAKE_CURRENT_BINARY_DIR}/version.cpp)

add_library(sua SHARED ${SRCS})
set_target_properties(sua PROPERTIES LINK_FLAGS -s)

add_executable(${PROJECT_NAME} ${SRCS} main.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS -s)

link_directories(${CMAKE_BINARY_DIR}/3rdparty/curl/lib)

target_link_libraries(${PROJECT_NAME}
    curl_lib
    paho-mqttpp3
    paho-mqtt3a
    gio_lib
    gobject_lib
    gmodule_lib
    glib_lib
    ffi_lib
    z_lib
    intl_lib
    ${SUA_PLATFORM_LIBS}
    ssl_lib
    crypto_lib
    sua
)

if(SUA_MEASURE_CODE_COVERAGE)
    target_link_libraries(sua gcov)
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
install(TARGETS sua LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

