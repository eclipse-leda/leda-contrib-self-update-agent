#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <chrono>
#include <thread>
#include <regex>

using ::testing::Return;
using ::testing::WithArgs;
using ::testing::Invoke;
using ::testing::_;

#include "Logger.h"
#include "SelfUpdateAgent.h"
#include "Utils/BundleChecker.h"
#include "Utils/JsonUtils.h"
#include "Mqtt/MqttMessagingProtocolJSON.h"
#include "Install/DummyRaucInstaller.h"

#include "MockFSM.h"
#include "MockDownloader.h"
#include "MockMqttProcessor.h"

using namespace std::chrono_literals;

namespace {

    static const std::string BUNDLE_10 = "bundle 1.0";
    static const std::string BUNDLE_11 = "bundle 1.1";
    static const std::string BUNDLE_RAUC_SETUP_FAILS = "rauc_setup_fails";

    static const std::string COMMAND_DOWNLOAD = "DOWNLOAD";
    static const std::string COMMAND_INSTALL  = "INSTALL";

    class MockRaucInstaller : public sua::DummyRaucInstaller {
    public:
        sua::TechCode installBundle(const std::string& input) override {
            if(input == BUNDLE_RAUC_SETUP_FAILS) {
                return sua::TechCode::InstallationFailed;
            }

            installedVersion = getBundleVersion(input);
            return sua::TechCode::OK;
        }

        std::string getBundleVersion() override {
            return installedVersion;
        }

        std::string getBundleVersion(const std::string& input) override {
            std::string       bundle;
            std::string       version;
            std::stringstream ss(input);
            ss >> bundle >> version;
            return version;
        }

        MOCK_METHOD(bool, succeeded, (), (override));

    private:
        std::string installedVersion = "1.0";
    };

    class TestSelfUpdateScenarios : public ::testing::Test {
    public:
        sua::Context & ctx() {
            return sua.context();
        }

        void start() {
            sua.start({testBrokerHost, testBrokerPort});
            std::this_thread::sleep_for(1s);
        }

        void triggerIdentify(const std::string & bundle, const std::string & version) {
            // clang-format off
            const std::string json = sua::jsonTemplate(R"(
                {
                    "activityId": "random-uuid-as-string",
                    "timestamp": 123456789,
                    "payload": {
                        "domains": [
                            {
                                "id": "self-update",
                                "components": [
                                    {
                                        "id": "os-image",
                                        "version": "{}",
                                        "config": [
                                            {
                                                "key": "image",
                                                "value": "{}"
                                            }
                                        ]
                                    }
                                ]
                            }
                        ]
                    }
                }
            )");
            // clang-format on

            execute(sua::IMqttProcessor::TOPIC_IDENTIFY, fmt::format(json, version, bundle));
        }

        void trigger(const std::string & action) {
            // clang-format off
            const std::string json = sua::jsonTemplate(R"(
                {
                    "activityId": "random-uuid-as-string",
                    "timestamp": 123456789,
                    "payload": {
                        "baseline": "BASELINE NAME",
                        "command": "{}"
                    }
                }
            )");
            // clang-format on

            execute(sua::IMqttProcessor::TOPIC_COMMAND, fmt::format(json, action));
        }

        void triggerCurrentStateRequest() {
            // clang-format off
            const std::string json = R"(
                {
                    "activityId": "random-uuid-as-string",
                    "timestamp": 123456789
                }
            )";
            // clang-format on

            execute(sua::IMqttProcessor::TOPIC_STATE_GET, json);
        }

        void execute(const std::string& topic, const std::string& json) {
            std::stringstream ss;
            ss << "mosquitto_pub";
            ss << " -h " << testBrokerHost;
            ss << " -t " << topic;
            ss << " -m '" << json << "'";
            std::system(ss.str().c_str());
            std::this_thread::sleep_for(1s);
        }

        void SetUp() override {
            sua::Logger::instance().init();
            sua::Logger::instance().setLogLevel(sua::Logger::Level::All);

            downloader = std::make_shared<MockDownloader>();
            ctx().downloaderAgent = downloader;

            fsm = std::make_shared<MockFSM>(sua.context(), visitedStates);
            ctx().stateMachine = fsm;

            installerAgent = std::make_shared<MockRaucInstaller>();
            ctx().installerAgent = installerAgent;
 
            mqttProcessor = std::make_shared<MockMqttProcessor>(sua.context(), sentMessages);
            ctx().mqttProcessor = mqttProcessor;

            ctx().messagingProtocol = std::make_shared<sua::MqttMessagingProtocolJSON>();
            ctx().bundleChecker     = std::make_shared<sua::BundleChecker>();
        }

        sua::SelfUpdateAgent sua;

        std::shared_ptr<MockFSM>           fsm;
        std::shared_ptr<MockMqttProcessor> mqttProcessor;
        std::shared_ptr<MockRaucInstaller> installerAgent;
        std::shared_ptr<MockDownloader>    downloader;

        std::string testBrokerHost = "localhost";
        int         testBrokerPort = 1883;

        std::vector<std::string> visitedStates;
        std::vector<std::string> expectedStates;

        std::vector<std::string> sentMessages;
        std::vector<std::string> expectedMessages;
    };

    TEST_F(TestSelfUpdateScenarios, doesNotConnect)
    {
        expectedStates = {"Uninitialized"};
        sua.init();

        EXPECT_EQ(visitedStates, expectedStates);
    }

    TEST_F(TestSelfUpdateScenarios, connectsAndSendsCurrentVersion)
    {
        expectedStates   = {"Uninitialized", "Connected"};
        expectedMessages = {"systemVersion"};

        sua.init();
        start();

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, receivesIdentifyRequestWithUnchandedVersion_skips)
    {
        expectedStates   = {"Uninitialized", "Connected", "Failed"};
        expectedMessages = {"systemVersion", "identifying", "skipped"};

        sua.init();
        start();

        triggerIdentify(BUNDLE_10, "1.0");

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, receivesCurrentStateRequest_sendsCurrentVersion)
    {
        expectedStates = {"Uninitialized", "Connected", "SendCurrentState", "Idle"};
        expectedMessages = {"systemVersion", "systemVersion"};

        sua.init();
        start();

        triggerCurrentStateRequest();

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, receivesIdentifyRequest_waitingForDownloadCommand)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading"};
        expectedMessages = {"systemVersion", "identifying", "identified"};

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.1");

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, receivesDownloadCommandAndDownloadFails_updateFails)
    {
        expectedStates = {"Uninitialized", "Connected", "Downloading", "Failed"};
        expectedMessages = {"systemVersion", "identifying", "identified", "downloadFailed"};

        EXPECT_CALL(*downloader, start(_)).WillOnce(Return(sua::TechCode::DownloadFailed));

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.1");
        trigger(COMMAND_DOWNLOAD);

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, receivesDownloadCommandAndDownloadedBundleVersionMismatch_rejectsUpdate)
    {
        expectedStates = {"Uninitialized", "Connected", "Downloading", "Failed"};
        expectedMessages = {"systemVersion", "identifying", "identified", "downloaded", "rejected"};

        EXPECT_CALL(*downloader, start(_)).WillOnce(Return(sua::TechCode::OK));

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.2");
        trigger(COMMAND_DOWNLOAD);

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, receivesDownloadCommandAndDownloadSucceeds_waitsForInstallCommand)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading", "Failed"};
        expectedMessages = {"systemVersion", "identifying", "identified", "downloaded", "rejected"};

        EXPECT_CALL(*downloader, start(_)).WillOnce(Return(sua::TechCode::OK));

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.1");
        trigger(COMMAND_DOWNLOAD);

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, receivesInstallCommandAndInstallSetupFails_sendsInstallFailed)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading", "Installing", "Failed"};
        expectedMessages = {"systemVersion", "identifying", "identified", "downloaded", "installFailed"};

        EXPECT_CALL(*downloader, start(_)).WillOnce(Return(sua::TechCode::OK));

        sua.init();
        start();

        triggerIdentify(BUNDLE_RAUC_SETUP_FAILS, "1.1");
        trigger(COMMAND_DOWNLOAD);
        trigger(COMMAND_INSTALL);
    }

    TEST_F(TestSelfUpdateScenarios, receivesInstallCommandAndInstallSucceeds_sendsInstalled)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading", "Installing", "Installed"};
        expectedMessages = {"systemVersion", "identifying", "identified", "downloaded", "installing", "installed", "currentState"};

        EXPECT_CALL(*downloader, start(_)).WillOnce(Return(sua::TechCode::OK));

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.1");
        trigger(COMMAND_DOWNLOAD);
        trigger(COMMAND_INSTALL);

        std::this_thread::sleep_for(2s);
    }

}

