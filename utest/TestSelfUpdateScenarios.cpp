#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <chrono>
#include <thread>
#include <regex>

using ::testing::Return;
using ::testing::_;

#include "Logger.h"
#include "SelfUpdateAgent.h"
#include "Utils/BundleChecker.h"
#include "Utils/JsonUtils.h"
#include "Mqtt/MqttMessagingProtocolJSON.h"
#include "Mqtt/MqttMessage.h"
#include "Install/DummyRaucInstaller.h"
#include "Download/Downloader.h"

#include "MockDownloader.h"
#include "MockFSM.h"
#include "MockMqttProcessor.h"

using namespace std::chrono_literals;
using M = sua::MqttMessage;

namespace {

    static const std::string BUNDLE_10 = "bundle 1.0";
    static const std::string BUNDLE_11 = "bundle 1.1";
    static const std::string BUNDLE_RAUC_SETUP_FAILS = "rauc_setup_fails 1.1";

    static const std::string COMMAND_DOWNLOAD = "DOWNLOAD";
    static const std::string COMMAND_UPDATE   = "UPDATE";
    static const std::string COMMAND_ACTIVATE = "ACTIVATE";
    static const std::string COMMAND_CLEANUP  = "CLEANUP";

    class MockRaucInstaller : public sua::DummyRaucInstaller {
    public:
        sua::TechCode installBundle(const std::string& input) override {
            const std::string bundle = getBundleName(input);

            if(bundle == BUNDLE_RAUC_SETUP_FAILS) {
                return sua::TechCode::InstallationFailed;
            }

            installedVersion = getBundleVersion(bundle);
            return sua::TechCode::OK;
        }

        std::string getBundleName(const std::string& input) {
            if((input == "/data/selfupdates/temp_file") && !bundleUnderTest.empty()) {
                return bundleUnderTest;
            }

            return input;
        }

        sua::SlotStatus getSlotStatus() override {
            sua::SlotStatus s;

            s["rootfs.0"]["state"  ] = "booted";
            s["rootfs.0"]["version"] = getBootedVersion();

            return s;
        }

        std::string getBootedVersion() override {
            return installedVersion;
        }

        std::string getBundleVersion(const std::string& input) override {
            if((input == "/data/selfupdates/temp_file") && !bundleUnderTest.empty()) {
                return getBundleVersion(bundleUnderTest);
            }

            std::string       bundle;
            std::string       version;
            std::stringstream ss(input);
            ss >> bundle >> version;
            return version;
        }

        MOCK_METHOD(bool, succeeded, (), (override));

        std::string installedVersion = "1.0";

        // succeeded download shadows bundle name by replacing it to /data/selfupdates/temp_file
        // this is a workaround to get the version from the bundle name
        // set it in all test cases doing install
        std::string bundleUnderTest;
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

        void triggerIdentify(const std::string & /*bundle*/, const std::string & version) {
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
                                                "value": "{}://localhost/bundle"
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

            execute(sua::IMqttProcessor::TOPIC_IDENTIFY, fmt::format(json, version, downloadProtocol));
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

            ctx().downloaderAgent = std::make_shared<sua::Downloader>(ctx());

            fsm = std::make_shared<MockFSM>(sua.context(), visitedStates);
            ctx().stateMachine = fsm;

            installerAgent = std::make_shared<MockRaucInstaller>();
            ctx().installerAgent = installerAgent;
 
            mqttProcessor = std::make_shared<MockMqttProcessor>(sua.context(), sentMessages);
            ctx().mqttProcessor = mqttProcessor;

            ctx().messagingProtocol = std::make_shared<sua::MqttMessagingProtocolJSON>();
            ctx().bundleChecker     = std::make_shared<sua::BundleChecker>();
        }

        void TearDown() override {
            mqttProcessor->stop();
        }

        sua::SelfUpdateAgent sua;

        std::shared_ptr<MockFSM>           fsm;
        std::shared_ptr<MockMqttProcessor> mqttProcessor;
        std::shared_ptr<MockRaucInstaller> installerAgent;

        std::string testBrokerHost = "localhost";
        int         testBrokerPort = 1883;

        std::string downloadProtocol = "https";

        std::string bundleUnderTest;

        std::vector<std::string> visitedStates;
        std::vector<std::string> expectedStates;

        std::vector<sua::MqttMessage> sentMessages;
        std::vector<sua::MqttMessage> expectedMessages;
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
        expectedMessages = {M::SystemVersion};

        sua.init();
        start();

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, receivesIdentifyRequestWithUnchandedVersion_endsInFailedState)
    {
        expectedStates   = {"Uninitialized", "Connected"};
        expectedMessages = {M::SystemVersion, M::Identifying, M::Skipped};

        sua.init();
        start();

        triggerIdentify(BUNDLE_10, "1.0");

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, receivesCurrentStateRequest_sendsCurrentVersion)
    {
        expectedStates = {"Uninitialized", "Connected", "SendCurrentState", "Idle"};
        expectedMessages = {M::SystemVersion, M::SystemVersion};

        sua.init();
        start();

        triggerCurrentStateRequest();

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, receivesIdentifyRequest_waitsForDownloadCommand)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading"};
        expectedMessages = {M::SystemVersion, M::Identifying, M::Identified};

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.1");

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, downloadFails_endsInFailedState)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading", "Failed"};
        expectedMessages = {M::SystemVersion, M::Identifying, M::Identified, M::DownloadFailed};

        auto downloader = std::make_shared<MockDownloader>();
        ctx().downloaderAgent = downloader;

        sua.init();
        start();

        EXPECT_CALL(*downloader, start(_)).WillOnce(Return(std::make_tuple(sua::TechCode::DownloadFailed, "")));

        triggerIdentify(BUNDLE_11, "1.1");
        trigger(COMMAND_DOWNLOAD);

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, downloadedBundleVersionMismatchWithSpec_endsInFailedState)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading", "Installing", "Failed"};
        expectedMessages = {M::SystemVersion, M::Identifying, M::Identified, M::Downloading, M::Downloaded, M::VersionChecking, M::Rejected};

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.2");
        trigger(COMMAND_DOWNLOAD);
        trigger(COMMAND_UPDATE);

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, downloadSucceeds_waitsForInstallCommand)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading", "Installing"};
        expectedMessages = {M::SystemVersion, M::Identifying, M::Identified, M::Downloading, M::Downloaded};

        installerAgent->bundleUnderTest = BUNDLE_11;

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.1");
        trigger(COMMAND_DOWNLOAD);

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, installSetupFails_endsInIdleState)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading", "Installing", "Failed"};
        expectedMessages = {M::SystemVersion, M::Identifying, M::Identified, M::Downloading, M::Downloaded, M::VersionChecking, M::InstallFailed};

        installerAgent->bundleUnderTest = BUNDLE_RAUC_SETUP_FAILS;

        sua.init();
        start();

        triggerIdentify(BUNDLE_RAUC_SETUP_FAILS, "1.1");
        trigger(COMMAND_DOWNLOAD);
        trigger(COMMAND_UPDATE);

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, installSucceeds_waitsInInstalledState)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading", "Installing", "Installed"};
        expectedMessages = {M::SystemVersion, M::Identifying, M::Identified, M::Downloading, M::Downloaded,
            M::VersionChecking, M::Installing, M::Installing, M::Installing, M::Installing, M::Installed,
            M::CurrentState};

        EXPECT_CALL(*installerAgent, succeeded()).WillOnce(Return(true));
        installerAgent->bundleUnderTest = BUNDLE_11;

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.1");
        trigger(COMMAND_DOWNLOAD);
        trigger(COMMAND_UPDATE);

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, installFails_endsInFailedState)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading", "Installing", "Failed"};
        expectedMessages = {M::SystemVersion, M::Identifying, M::Identified, M::Downloading, M::Downloaded,
            M::VersionChecking, M::Installing, M::Installing, M::Installing, M::Installing, M::InstallFailed};

        EXPECT_CALL(*installerAgent, succeeded()).WillOnce(Return(false));
        installerAgent->bundleUnderTest = BUNDLE_11;

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.1");
        trigger(COMMAND_DOWNLOAD);
        trigger(COMMAND_UPDATE);

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, activationSucceeds_endsInIdleState)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading", "Installing", "Installed", "Activating", "Cleaning", "Idle"};
        expectedMessages = {M::SystemVersion, M::Identifying, M::Identified, M::Downloading, M::Downloaded, M::VersionChecking,
            M::Installing, M::Installing, M::Installing, M::Installing, M::Installed, M::CurrentState,
            M::Activating, M::Activated, M::Cleaned, M::Complete};

        EXPECT_CALL(*installerAgent, succeeded()).WillOnce(Return(true));
        installerAgent->bundleUnderTest = BUNDLE_11;

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.1");
        trigger(COMMAND_DOWNLOAD);
        trigger(COMMAND_UPDATE);
        trigger(COMMAND_ACTIVATE);
        trigger(COMMAND_CLEANUP);

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

    TEST_F(TestSelfUpdateScenarios, downloadViaHttpFails_endsInFailedState)
    {
        expectedStates   = {"Uninitialized", "Connected", "Downloading", "Failed"};
        expectedMessages = {M::SystemVersion, M::Identifying, M::Identified, M::DownloadFailed};

        downloadProtocol = "http";

        sua.init();
        start();

        triggerIdentify(BUNDLE_11, "1.2");
        trigger(COMMAND_DOWNLOAD);

        EXPECT_EQ(visitedStates, expectedStates);
        EXPECT_EQ(sentMessages, expectedMessages);
    }

}
