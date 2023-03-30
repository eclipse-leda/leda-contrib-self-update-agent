#include "gtest/gtest.h"
#include "gmock/gmock.h"

using ::testing::Return;
using ::testing::WithArg;
using ::testing::Invoke;
using ::testing::_;

#include "Logger.h"
#include "SelfUpdateAgent.h"
#include "Utils/BundleChecker.h"

#include "MockFSM.h"
#include "MockDownloader.h"
#include "MockMqttMessagingProtocol.h"
#include "MockMqttProcessor.h"
#include "MockRaucInstaller.h"

namespace {

    class TestSelfUpdateScenarios : public ::testing::Test {
    public:
        TestSelfUpdateScenarios() {
        }

        TestSelfUpdateScenarios & test() {
            return *this;
        }

        void willSend(const std::string & topic, const std::string & name, const bool retained = false) {
            EXPECT_CALL(*mqttProcessor, send(topic, _, retained))
                .Times(1)
                .RetiresOnSaturation();

            EXPECT_CALL(*messagingProtocol, createMessage(_, name, _))
                .Times(1)
                .RetiresOnSaturation();
        }

        void willSend(const std::string & topic, const std::string & name, const std::string & message) {
            EXPECT_CALL(*mqttProcessor, send(topic, _, false))
                .Times(1)
                .RetiresOnSaturation();

            EXPECT_CALL(*messagingProtocol, createMessage(_, name, message))
                .Times(1)
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

        void willTransitTo(const std::string & name) {
            EXPECT_CALL(*fsm, transitTo(name))
                .WillOnce(
                    WithArg<0>(Invoke(fsm.get(), &MockFSM::native_transitTo)
                ))
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

        void SetUp() override {
            if(WEXITSTATUS(std::system("systemctl status mosquitto > /dev/null")) != 0) {
                FAIL() << "mosquitto service is not running on this machine!";
            }

            sua::Logger::instance().setLogLevel(sua::Logger::Level::None);

            downloader = std::make_shared<MockDownloader>();
            ctx().downloaderAgent = downloader;

            fsm = std::make_shared<MockFSM>(sua.context());
            ctx().stateMachine = fsm;

            messagingProtocol = std::make_shared<MockMqttMessagingProtocol>();
            ctx().messagingProtocol = messagingProtocol;

            mqttProcessor = std::make_shared<MockMqttProcessor>();
            ctx().mqttProcessor = mqttProcessor;

            installerAgent = std::make_shared<MockRaucInstaller>();
            ctx().installerAgent = installerAgent;
        
            ctx().bundleChecker = std::make_shared<sua::BundleChecker>();
        }

        sua::SelfUpdateAgent sua;

        std::shared_ptr<MockFSM>                   fsm;
        std::shared_ptr<MockMqttMessagingProtocol> messagingProtocol;
        std::shared_ptr<MockMqttProcessor>         mqttProcessor;
        std::shared_ptr<MockRaucInstaller>         installerAgent;
        std::shared_ptr<MockDownloader>            downloader;
    };

    TEST_F(TestSelfUpdateScenarios, doNotConnect)
    {
        currentVersionIs("1.0");
        willTransitTo("Uninitialized");
        
        sua.init();
    }

    TEST_F(TestSelfUpdateScenarios, sendsCurrentVersion)
    {
        currentVersionIs("1.0");
        willTransitTo("Uninitialized");
        willTransitTo("Connected");
        willSend("selfupdate/currentstate", "systemVersion", true);

        sua.init();
        ctx().stateMachine->handleEvent(sua::FotaEvent::ConnectivityEstablished);
    }

    TEST_F(TestSelfUpdateScenarios, receivesUnchandedVersion_skipsUpdate)
    {
        currentVersionIs("1.0");
        willTransitTo("Uninitialized");

        willTransitTo("Connected");
        willSend("selfupdate/currentstate", "systemVersion", true);
        currentVersionIs("1.0");
        willSend("selfupdate/desiredstatefeedback", "identifying");
        willSend("selfupdate/desiredstatefeedback", "skipped");

        willTransitTo("Failed");
        willTransitTo("Idle");

        sua.init();
        ctx().desiredState.bundleVersion = "1.0";
        ctx().stateMachine->handleEvent(sua::FotaEvent::ConnectivityEstablished);
        ctx().stateMachine->handleEvent(sua::FotaEvent::Start);
    }

    TEST_F(TestSelfUpdateScenarios, receivesCurrentStateRequest_sendsCurrentVersion)
    {
        currentVersionIs("1.0");
        willTransitTo("Uninitialized");
        
        willTransitTo("Connected");
        willSend("selfupdate/currentstate", "systemVersion", true);

        willTransitTo("SendCurrentState");
        willSend("selfupdate/currentstate", "systemVersion", true);

        willTransitTo("Idle");

        sua.init();
        ctx().stateMachine->handleEvent(sua::FotaEvent::ConnectivityEstablished);
        ctx().stateMachine->handleEvent(sua::FotaEvent::GetCurrentState);
    }

    TEST_F(TestSelfUpdateScenarios, receivesUpdateRequestAndDownloadFails_rejectsUpdate)
    {
        currentVersionIs("1.0");
        willTransitTo("Uninitialized");

        willTransitTo("Connected");
        willSend("selfupdate/currentstate", "systemVersion", true);
        currentVersionIs("1.0");
        willSend("selfupdate/desiredstatefeedback", "identifying");
        willSend("selfupdate/desiredstatefeedback", "identified");

        willTransitTo("Downloading");
        downloadWillFail();
        willSend("selfupdate/desiredstatefeedback", "downloadFailed");

        willTransitTo("Failed");
        willTransitTo("Idle");

        sua.init();
        ctx().desiredState.bundleVersion = "1.1";
        ctx().stateMachine->handleEvent(sua::FotaEvent::ConnectivityEstablished);
        ctx().stateMachine->handleEvent(sua::FotaEvent::Start);
    }

    TEST_F(TestSelfUpdateScenarios, receivesUpdateRequestAndBundleVersionMismatch_rejectsUpdate)
    {
        willTransitTo("Uninitialized");
        currentVersionIs("1.0");

        willTransitTo("Connected");
        willSend("selfupdate/currentstate", "systemVersion", true);
        currentVersionIs("1.0");
        willSend("selfupdate/desiredstatefeedback", "identifying");
        willSend("selfupdate/desiredstatefeedback", "identified");

        willTransitTo("Downloading");
        downloadWillSucceed();
        bundleVersionIs("1.2");
        willSend("selfupdate/desiredstatefeedback", "downloaded");
        willSend("selfupdate/desiredstatefeedback", "rejected");

        willTransitTo("Failed");
        willTransitTo("Idle");

        sua.init();
        ctx().desiredState.bundleVersion = "1.1";
        ctx().stateMachine->handleEvent(sua::FotaEvent::ConnectivityEstablished);
        ctx().stateMachine->handleEvent(sua::FotaEvent::Start);
    }

    TEST_F(TestSelfUpdateScenarios, receivesUpdateRequestAndInstallSetupFails_sendsInstallFailed)
    {
        willTransitTo("Uninitialized");
        currentVersionIs("1.0");

        willTransitTo("Connected");
        willSend("selfupdate/currentstate", "systemVersion", true);
        currentVersionIs("1.0");
        willSend("selfupdate/desiredstatefeedback", "identifying");
        willSend("selfupdate/desiredstatefeedback", "identified");

        willTransitTo("Downloading");
        downloadWillSucceed();
        bundleVersionIs("1.1");
        willSend("selfupdate/desiredstatefeedback", "downloaded");

        willTransitTo("Installing");
        installSetupWillFail();
        lastErrorWillBe("test");
        willSend("selfupdate/desiredstatefeedback", "installFailed");

        willTransitTo("Failed");
        willTransitTo("Idle");

        sua.init();
        ctx().desiredState.bundleVersion = "1.1";
        ctx().stateMachine->handleEvent(sua::FotaEvent::ConnectivityEstablished);
        ctx().stateMachine->handleEvent(sua::FotaEvent::Start);
    }

    TEST_F(TestSelfUpdateScenarios, receivesUpdateRequestAndInstallSucceeds_sendsInstalled)
    {
        willTransitTo("Uninitialized");
        currentVersionIs("1.0");

        willTransitTo("Connected");
        willSend("selfupdate/currentstate", "systemVersion", true);
        currentVersionIs("1.0");
        willSend("selfupdate/desiredstatefeedback", "identifying");
        willSend("selfupdate/desiredstatefeedback", "identified");

        willTransitTo("Downloading");
        downloadWillSucceed();
        bundleVersionIs("1.1");
        willSend("selfupdate/desiredstatefeedback", "downloaded");

        willTransitTo("Installing");
        installSetupWillSucceed();
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

        willTransitTo("Installed");
        willSend("selfupdate/desiredstatefeedback", "currentState");

        willTransitTo("Idle");

        sua.init();
        ctx().desiredState.bundleVersion = "1.1";
        ctx().stateMachine->handleEvent(sua::FotaEvent::ConnectivityEstablished);
        ctx().stateMachine->handleEvent(sua::FotaEvent::Start);
    }

}
