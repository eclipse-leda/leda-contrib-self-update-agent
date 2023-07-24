add_executable(TestSelfUpdateAgent
    TestDispatcher.cpp
    TestDownloader.cpp
    TestFSM.cpp
    TestLogger.cpp
    TestMqttMessagingProtocolJSON.cpp
    TestSelfUpdateScenarios.cpp
    TestServerAddressParser.cpp
    TestStateFactory.cpp
    Utils.cpp
)

include_directories(
    ../src
    ${CMAKE_SOURCE_DIR}/3rdparty/spdlog/include
    ${CMAKE_SOURCE_DIR}/3rdparty/nlohmann-json/include
)

target_link_libraries(
    TestSelfUpdateAgent
    sua
    curl_lib
    paho-mqttpp3
    paho-mqtt3a
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
