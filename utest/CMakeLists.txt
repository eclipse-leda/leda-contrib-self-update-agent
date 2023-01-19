add_executable(TestSelfUpdateAgent
    TestDispatcher.cpp
    TestFSM.cpp
    TestMqttMessagingProtocolYAML.cpp
    TestMqttMessagingProtocolJSON.cpp
    TestServerAddressParser.cpp
    Utils.cpp
)

include_directories(
    ../src
    ${CMAKE_SOURCE_DIR}/3rdparty/spdlog/include
)

target_link_libraries(
    TestSelfUpdateAgent
    sua
    curl_lib
    paho-mqttpp3
    paho-mqtt3a
    mini-yaml
    gio_lib
    gobject_lib
    gmodule_lib
    glib_lib
    ffi_lib
    z_lib
    ${SUA_PLATFORM_LIBS}
    ssl_lib
    crypto_lib
    gtest_main
    gmock
)

install(TARGETS TestSelfUpdateAgent RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/utest)
