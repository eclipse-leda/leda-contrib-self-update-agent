#include "gtest/gtest.h"

#include "Mqtt/MqttMessagingProtocolYAML.h"
#include "Context.h"
#include "Utils.h"

namespace {

    class TestMessagingProtocolYAML : public ::testing::Test {
    public:
        sua::Context ctx;

        sua::CurrentState& c = ctx.currentState;
        sua::DesiredState& d = ctx.desiredState;

        void SetUp() override {
            // clang-format off
            c.version                  = "1.0";
            d.bundleDownloadUrl        = "http://url";
            d.bundleVersion            = "1.0";
            d.metadata["apiVersion"  ] = "sdv.eclipse.org/v1";
            d.metadata["kind"        ] = "SelfUpdateBundle";
            d.metadata["name"        ] = "self-update-bundle-example";
            d.metadata["bundleTarget"] = "base";
            d.metadata["bundleName"  ] = "arm64-bundle";
            // clang-format on
        };
    };

    TEST_F(TestMessagingProtocolYAML, readDesiredState)
    {
        // clang-format off
        const std::string input = R"(
            apiVersion: "sdv.eclipse.org/v1"
            kind: SelfUpdateBundle
            metadata:
                name: self-update-bundle-example
            spec:
                bundleDownloadUrl: http://url
                bundleName: arm64-bundle
                bundleTarget: base
                bundleVersion: 1.0
        )";
        // clang-format on

        const sua::DesiredState s = sua::MqttMessagingProtocolYAML().readDesiredState(input);

        EXPECT_EQ(s.bundleDownloadUrl, "http://url");
        EXPECT_EQ(s.bundleVersion, "1.0");
        EXPECT_EQ(s.metadata.at("apiVersion"), "sdv.eclipse.org/v1");
        EXPECT_EQ(s.metadata.at("kind"), "SelfUpdateBundle");
        EXPECT_EQ(s.metadata.at("name"), "self-update-bundle-example");
        EXPECT_EQ(s.metadata.at("bundleTarget"), "base");
        EXPECT_EQ(s.metadata.at("bundleName"), "arm64-bundle");
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_systemVersion)
    {
        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "systemVersion");

        // clang-format off
        const std::string expected = R"(
            apiVersion: sdv.eclipse.org/v1
            kind: SelfUpdateBundle
            metadata: 
                name: "self-update-bundle-example"
            spec: 
                bundleVersion: 1.0
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_identifying)
    {
        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "identifying");
        EXPECT_EQ(result, "");
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_identified)
    {
        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "identified");
        EXPECT_EQ(result, "");
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_identifiedAndSkipped)
    {
        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "identifiedAndSkipped");
        EXPECT_EQ(result, "");
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_completedAndSkipped)
    {
        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "completedAndSkipped");
        EXPECT_EQ(result, "");
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_downloading)
    {
        d.downloadBytesDownloaded    = 1048576;
        d.downloadBytesTotal         = 10485760;
        d.downloadProgressPercentage = 10;

        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "downloading");

        // clang-format off
        const std::string expected = R"(
            apiVersion: sdv.eclipse.org/v1
            kind: SelfUpdateBundle
            metadata:
                name: "self-update-bundle-example"
            spec:
                bundleDownloadUrl: "http://url"
                bundleName: "arm64-bundle"
                bundleTarget: base
                bundleVersion: 1.0
            state:
                message: Downloading 10.0 MiB...
                name: downloading
                progress: 10
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_downloaded)
    {
        d.downloadBytesDownloaded    = 1048576;
        d.downloadBytesTotal         = 10485760;
        d.downloadProgressPercentage = 10;

        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "downloaded");

        // clang-format off
        const std::string expected = R"(
            apiVersion: sdv.eclipse.org/v1
            kind: SelfUpdateBundle
            metadata:
                name: "self-update-bundle-example"
            spec:
                bundleDownloadUrl: "http://url"
                bundleName: "arm64-bundle"
                bundleTarget: base
                bundleVersion: 1.0
            state:
                message: Downloaded 10.0 MiB...
                name: downloading
                progress: 100
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_installing)
    {
        d.installProgressPercentage = 42;

        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "installing");

        // clang-format off
        const std::string expected = R"(
            apiVersion: sdv.eclipse.org/v1
            kind: SelfUpdateBundle
            metadata:
                name: "self-update-bundle-example"
            spec:
                bundleDownloadUrl: "http://url"
                bundleName: "arm64-bundle"
                bundleTarget: base
                bundleVersion: 1.0
            state:
                message: RAUC install...
                name: installing
                progress: 42
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_installed)
    {
        d.installProgressPercentage = 100;

        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "installed");

        // clang-format off
        const std::string expected = R"(
            apiVersion: sdv.eclipse.org/v1
            kind: SelfUpdateBundle
            metadata:
                name: "self-update-bundle-example"
            spec:
                bundleDownloadUrl: "http://url"
                bundleName: "arm64-bundle"
                bundleTarget: base
                bundleVersion: 1.0
            state:
                message: RAUC install completed...
                name: installed
                progress: 100
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_currentState)
    {
        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "currentState");

        // clang-format off
        const std::string expected = R"(
            apiVersion: sdv.eclipse.org/v1
            kind: SelfUpdateBundle
            metadata:
                name: "self-update-bundle-example"
            spec:
                bundleDownloadUrl: "http://url"
                bundleName: "arm64-bundle"
                bundleTarget: base
                bundleVersion: 1.0
            state:
                message: Idle
                name: idle
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_downloadFailed)
    {
        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "downloadFailed");

        // clang-format off
        const std::string expected = R"(
            apiVersion: sdv.eclipse.org/v1
            kind: SelfUpdateBundle
            metadata:
                name: "self-update-bundle-example"
            spec:
                bundleDownloadUrl: "http://url"
                bundleName: "arm64-bundle"
                bundleTarget: base
                bundleVersion: 1.0
            state:
                message: Download failed
                name: failed
                techCode: 1001
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_invalidBundle)
    {
        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "invalidBundle");

        // clang-format off
        const std::string expected = R"(
            apiVersion: sdv.eclipse.org/v1
            kind: SelfUpdateBundle
            metadata:
                name: "self-update-bundle-example"
            spec:
                bundleDownloadUrl: "http://url"
                bundleName: "arm64-bundle"
                bundleTarget: base
                bundleVersion: 1.0
            state:
                message: Invalid bundle
                name: failed
                techCode: 2001
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_installFailed)
    {
        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "installFailed");

        // clang-format off
        const std::string expected = R"(
            apiVersion: sdv.eclipse.org/v1
            kind: SelfUpdateBundle
            metadata:
                name: "self-update-bundle-example"
            spec:
                bundleDownloadUrl: "http://url"
                bundleName: "arm64-bundle"
                bundleTarget: base
                bundleVersion: 1.0
            state:
                message: Install failed
                name: failed
                techCode: 3001
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolYAML, createMessage_updateRejected)
    {
        const std::string result = sua::MqttMessagingProtocolYAML().createMessage(ctx, "updateRejected");

        // clang-format off
        const std::string expected = R"(
            apiVersion: sdv.eclipse.org/v1
            kind: SelfUpdateBundle
            metadata:
                name: "self-update-bundle-example"
            spec:
                bundleDownloadUrl: "http://url"
                bundleName: "arm64-bundle"
                bundleTarget: base
                bundleVersion: 1.0
            state:
                message: Update rejected
                name: failed
                techCode: 4001
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

}
