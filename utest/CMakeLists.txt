add_executable(TestSDVSelfUpdateAgent
    TestMessageSerializer.cpp
    TestMessageDeserializer.cpp
    TestDispatcher.cpp
    ../src/Mqtt/MqttMessageSerializer.cpp
    ../src/Mqtt/MqttMessageDeserializer.cpp
    ../src/Patterns/Dispatcher.cpp
)
include_directories(
    ../src
    ${CMAKE_SOURCE_DIR}/3rdparty/mini-yaml
)

target_link_libraries(TestSDVSelfUpdateAgent gtest_main mini-yaml)