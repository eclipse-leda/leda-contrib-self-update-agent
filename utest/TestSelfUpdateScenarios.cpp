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
#include "FSM/FSM.h"
#include "Utils/BundleChecker.h"
#include "Mqtt/MqttMessagingProtocolJSON.h"

#include "MockDownloader.h"
#include "MockMqttProcessor.h"
#include "MockRaucInstaller.h"

using namespace std::chrono_literals;

namespace {

    std::string jsonTemplate(std::string tpl)
    {
        // required because lib-fmt expects curly brackets escaped as {{ and }}
        // to properly handle placeholders 3 steps are done:
        //   { -> {{
        //   } -> }}
        // this makes placeholder {} look like {{}}
        //   {{}} -> {}

        // escape opening
        tpl = std::regex_replace(tpl, std::regex("\\{"), "{{");
        // espace closing
        tpl = std::regex_replace(tpl, std::regex("\\}"), "}}");
        // unescape placeholder
        tpl = std::regex_replace(tpl, std::regex("\\{\\{\\}\\}"), "{}");

        return tpl;
    }

    class TestSelfUpdateScenarios : public ::testing::Test {
    public:
        void willSend(const std::string & topic, const std::string & name, const std::string & message = "", const bool retained = false) {
            EXPECT_CALL(*mqttProcessor, send(topic, name, message, retained))
                .WillOnce(
                    WithArgs<0, 1, 2, 3>(
                        Invoke(mqttProcessor.get(), &MockMqttProcessor::native_send)
                    )
                )
                .RetiresOnSaturation();
        }

        void currentVersionIs(const std::string & version) {
            EXPECT_CALL(*installerAgent, getBundleVersion())
                .WillOnce(Return(version))
                .RetiresOnSaturation();
        }

        void bundleVersionIs(const std::string & version) {
            EXPECT_CALL(*installerAgent, getBundleVersion(_))
                .WillOnce(Return(version))
                .RetiresOnSaturation();
        }

        void downloadWillFail() {
            EXPECT_CALL(*downloader, start(_))
                .WillOnce(Return(sua::TechCode::DownloadFailed))
                .RetiresOnSaturation();
        }

        void downloadWillSucceed() {
            EXPECT_CALL(*downloader, start(_))
                .WillOnce(Return(sua::TechCode::OK))
                .RetiresOnSaturation();
        }

        void installSetupWillFail()
        {
            EXPECT_CALL(*installerAgent, installBundle(_))
                .WillOnce(Return(sua::TechCode::InstallationFailed))
                .RetiresOnSaturation();
        }

        void installSetupWillSucceed()
        {
            EXPECT_CALL(*installerAgent, installBundle(_))
                .WillOnce(Return(sua::TechCode::OK))
                .RetiresOnSaturation();
        }

        void installStatusWillBeSuccess()
        {
            EXPECT_CALL(*installerAgent, succeeded())
                .WillOnce(Return(true))
                .RetiresOnSaturation();
        }

        void installProgressWillBe(const int value)
        {
            EXPECT_CALL(*installerAgent, getInstallProgress())
                .WillOnce(Return(value))
                .RetiresOnSaturation();
        }

        void installStatusWillBe(const bool value)
        {
            EXPECT_CALL(*installerAgent, installing())
                .WillOnce(Return(value))
                .RetiresOnSaturation();
        }

        void lastErrorWillBe(const std::string & text)
        {
            EXPECT_CALL(*installerAgent, getLastError())
                .WillOnce(Return(text))
                .RetiresOnSaturation();
        }

        sua::Context & ctx() {
            return sua.context();
        }

        void start() {
            sua.start({testBrokerHost, testBrokerPort});
            std::this_thread::sleep_for(1s);
        }

        void triggerUpdate(const std::string & version) {
            // clang-format off
            const std::string json = jsonTemplate(R"(
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
                                                "value": "http://example.com/downloads/os-image-1.1.bin"
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

            execute(sua::IMqttProcessor::TOPIC_START, fmt::format(json, version));
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

            fsm = std::make_shared<sua::FSM>(sua.context());
            ctx().stateMachine = fsm;

            installerAgent = std::make_shared<MockRaucInstaller>();
            ctx().installerAgent = installerAgent;
 
            mqttProcessor = std::make_shared<MockMqttProcessor>(sua.context());
            ctx().mqttProcessor = mqttProcessor;

            ctx().messagingProtocol = std::make_shared<sua::MqttMessagingProtocolJSON>();
            ctx().bundleChecker     = std::make_shared<sua::BundleChecker>();

            currentVersionIs("1.0");
            sua.init();
        }

        sua::SelfUpdateAgent sua;

        std::shared_ptr<sua::FSM>          fsm;
        std::shared_ptr<MockMqttProcessor> mqttProcessor;
        std::shared_ptr<MockRaucInstaller> installerAgent;
        std::shared_ptr<MockDownloader>    downloader;

        std::string testBrokerHost = "localhost";
        int         testBrokerPort = 1883;
    };

    TEST_F(TestSelfUpdateScenarios, doNotConnect)
    {
        EXPECT_EQ(fsm->activeState(), "Uninitialized");
    }

    TEST_F(TestSelfUpdateScenarios, sendsCurrentVersion)
    {
        willSend("selfupdate/currentstate", "systemVersion", "", true);

        start();
    }

    TEST_F(TestSelfUpdateScenarios, receivesUnchandedVersion_skipsUpdate)
    {
        willSend("selfupdate/currentstate", "systemVersion", "", true);
        willSend("selfupdate/desiredstatefeedback", "identifying");
        willSend("selfupdate/desiredstatefeedback", "skipped");

        currentVersionIs("1.0");

        start();
        triggerUpdate("1.0");
    }

    TEST_F(TestSelfUpdateScenarios, receivesCurrentStateRequest_sendsCurrentVersion)
    {
        willSend("selfupdate/currentstate", "systemVersion", "", true);
        willSend("selfupdate/currentstate", "systemVersion", "", true);

        start();
        triggerCurrentStateRequest();
    }

    TEST_F(TestSelfUpdateScenarios, receivesUpdateRequestAndDownloadFails_rejectsUpdate)
    {
        willSend("selfupdate/currentstate", "systemVersion", "", true);
        willSend("selfupdate/desiredstatefeedback", "identifying");
        willSend("selfupdate/desiredstatefeedback", "identified");
        willSend("selfupdate/desiredstatefeedback", "downloadFailed");

        downloadWillFail();
        willSend("selfupdate/desiredstatefeedback", "downloadFailed");

        currentVersionIs("1.0");

        start();
        triggerUpdate("1.1");
    }

    TEST_F(TestSelfUpdateScenarios, receivesUpdateRequestAndBundleVersionMismatch_rejectsUpdate)
    {
        willSend("selfupdate/currentstate", "systemVersion", "", true);
        willSend("selfupdate/desiredstatefeedback", "identifying");
        willSend("selfupdate/desiredstatefeedback", "identified");
        willSend("selfupdate/desiredstatefeedback", "downloaded");
        willSend("selfupdate/desiredstatefeedback", "rejected");

        downloadWillSucceed();

        bundleVersionIs("1.2");
        currentVersionIs("1.0");

        start();
        triggerUpdate("1.1");
    }

    TEST_F(TestSelfUpdateScenarios, receivesUpdateRequestAndInstallSetupFails_sendsInstallFailed)
    {
        willSend("selfupdate/currentstate", "systemVersion", "", true);
        willSend("selfupdate/desiredstatefeedback", "identifying");
        willSend("selfupdate/desiredstatefeedback", "identified");
        willSend("selfupdate/desiredstatefeedback", "downloaded");
        willSend("selfupdate/desiredstatefeedback", "installFailed");

        downloadWillSucceed();
        bundleVersionIs("1.1");
        willSend("selfupdate/desiredstatefeedback", "downloaded");

        installSetupWillFail();
        lastErrorWillBe("test");
        willSend("selfupdate/desiredstatefeedback", "installFailed");

        bundleVersionIs("1.1");
        currentVersionIs("1.0");

        start();
        triggerUpdate("1.1");
    }

    TEST_F(TestSelfUpdateScenarios, receivesUpdateRequestAndInstallSucceeds_sendsInstalled)
    {
        willSend("selfupdate/currentstate", "systemVersion", "", true);
        willSend("selfupdate/desiredstatefeedback", "identifying");
        willSend("selfupdate/desiredstatefeedback", "identified");
        willSend("selfupdate/desiredstatefeedback", "downloaded");
        willSend("selfupdate/desiredstatefeedback", "installing");
        willSend("selfupdate/desiredstatefeedback", "installing");
        willSend("selfupdate/desiredstatefeedback", "installed");
        willSend("selfupdate/desiredstatefeedback", "currentState");

        currentVersionIs("1.0");

        downloadWillSucceed();
        installSetupWillSucceed();

        bundleVersionIs("1.1");
 
        // gmock is checking expecations in reverse order
        installStatusWillBe(false); // completed (!installing)
        installStatusWillBe(true); // installing
        installStatusWillBe(true); // installing
        installProgressWillBe(99);
        installProgressWillBe(50);
        installStatusWillBeSuccess();
        willSend("selfupdate/desiredstatefeedback", "installing");
        willSend("selfupdate/desiredstatefeedback", "installing");
        willSend("selfupdate/desiredstatefeedback", "installed");

        start();
        triggerUpdate("1.1");
    }

}
