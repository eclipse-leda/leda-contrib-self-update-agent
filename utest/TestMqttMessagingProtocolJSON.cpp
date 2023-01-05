#include "gtest/gtest.h"

#include "Mqtt/MqttMessagingProtocolJSON.h"
#include "Context.h"
#include "Utils.h"

namespace {

    class ProtocolJSON : public sua::MqttMessagingProtocolJSON {
    protected:
        uint64_t epochTime() const {
            return 42;
        }
    };

    class TestMessagingProtocolJSON : public ::testing::Test {
    public:
        sua::Context ctx;

        sua::CurrentState& c = ctx.currentState;
        sua::DesiredState& d = ctx.desiredState;

        void SetUp() override {
            c.version = "1.0";

            d.activityId        = "id";
            d.bundleVersion     = "1.0";
            d.bundleDownloadUrl = "url";
        }
    };

    TEST_F(TestMessagingProtocolJSON, readDesiredState)
    {
        // clang-format off
        const std::string input = R"(
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
                                    "version": "1.1",
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
        )";
        // clang-format on

        const sua::DesiredState s = ProtocolJSON().readDesiredState(input);

        EXPECT_EQ(s.activityId, "random-uuid-as-string");
        EXPECT_EQ(s.bundleDownloadUrl, "http://example.com/downloads/os-image-1.1.bin");
        EXPECT_EQ(s.bundleVersion, "1.1");
    }

    TEST_F(TestMessagingProtocolJSON, readCurrentStateRequest)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "random-uuid-as-string",
                "timestamp": 123456789
            }
        )";
        // clang-format on

        const sua::DesiredState s = ProtocolJSON().readCurrentStateRequest(input);

        EXPECT_EQ(s.activityId, "random-uuid-as-string");
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_systemVersion)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, "systemVersion");

        // clang-format off
        const std::string expected = R"(
            {
                "timestamp": 42,
                "payload": {
                    "domains": [
                        {
                            "id": "self-update",
                            "components": [
                                {
                                    "id": "os-image",
                                    "version": "1.0"
                                }
                            ]
                        }
                    ]
                }
            }
        )";
        // clang-format on
        
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_identifying)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, "identifying");

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "IDENTIFYING",
                    "message": "Self-update agent has received new desired state request and is evaluating it.",
                    "actions": []
                }
            }
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_identified)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, "identified");

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "IDENTIFIED",
                    "message": "Self-update agent is about to perform an OS image update.",
                    "actions": []
                }
            }
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_skipped)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, "skipped");

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "COMPLETE",
                    "message": "Current OS image is equal to the target one from desired state.",
                    "actions": []
                }
            }
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_rejected)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, "rejected");

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "INCOMPLETE",
                    "message": "Update rejected.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "UPDATE_FAILURE",
                            "progress": 0,
                            "message": "Bundle version does not match version in desired state request."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_downloading)
    {
        d.downloadBytesDownloaded    = 1048576;
        d.downloadBytesTotal         = 10485760;
        d.downloadProgressPercentage = 10;

        const std::string result = ProtocolJSON().createMessage(ctx, "downloading");

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "RUNNING",
                    "message": "Self-update agent is performing an OS image update.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "DOWNLOADING",
                            "progress": 10,
                            "message": "Downloading 10.0 MiB..."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_downloaded)
    {
        d.downloadBytesDownloaded    = 1048576;
        d.downloadBytesTotal         = 10485760;
        d.downloadProgressPercentage = 10;

        const std::string result = ProtocolJSON().createMessage(ctx, "downloaded");

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "RUNNING",
                    "message": "Self-update agent is performing an OS image update.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "DOWNLOAD_SUCCESS",
                            "progress": 100,
                            "message": "Downloaded 10.0 MiB..."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_downloadFailed)
    {
        d.downloadProgressPercentage = 66;

        const std::string result = ProtocolJSON().createMessage(ctx, "downloadFailed");

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "INCOMPLETE",
                    "message": "Download failed.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "UPDATE_FAILURE",
                            "progress": 66,
                            "message": "Download failed."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_installing)
    {
        d.installProgressPercentage = 42;

        const std::string result = ProtocolJSON().createMessage(ctx, "installing");

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "RUNNING",
                    "message": "Self-update agent is performing an OS image update.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "UPDATING",
                            "progress": 42,
                            "message": "RAUC install..."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_installed)
    {
        d.installProgressPercentage = 100;

        const std::string result = ProtocolJSON().createMessage(ctx, "installed");

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "COMPLETE",
                    "message": "Self-update completed, reboot required.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "UPDATE_SUCCESS",
                            "progress": 100,
                            "message": "Writing partition completed, reboot required."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_installFailed)
    {
        d.installProgressPercentage = 66;

        const std::string result = ProtocolJSON().createMessage(ctx, "installFailed");

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "INCOMPLETE",
                    "message": "Install failed.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "UPDATE_FAILURE",
                            "progress": 66,
                            "message": "Writing partition failed."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_currentState)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, "currentState");

        EXPECT_EQ_MULTILINE(result, "");
    }

}
