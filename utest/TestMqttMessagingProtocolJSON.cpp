#include "gtest/gtest.h"

#include "Mqtt/MqttMessagingProtocolJSON.h"
#include "Mqtt/MqttMessage.h"
#include "Context.h"
#include "Utils.h"
#include  "nlohmann/json.hpp"

#include "version.h"

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

        void validateJsonSyntax(const std::string & input) {
            std::stringstream ss(input);
            nlohmann::json json = nlohmann::json::parse(ss);
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

        EXPECT_NO_THROW(validateJsonSyntax(input));
        const sua::DesiredState s = ProtocolJSON().readDesiredState(input);

        EXPECT_EQ(s.activityId, "random-uuid-as-string");
        EXPECT_EQ(s.bundleDownloadUrl, "http://example.com/downloads/os-image-1.1.bin");
        EXPECT_EQ(s.bundleVersion, "1.1");
    }

    TEST_F(TestMessagingProtocolJSON, readDesiredState_activityIdIsEmpty_throwsLogicError)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "",
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

        EXPECT_NO_THROW(validateJsonSyntax(input));
        EXPECT_THROW(ProtocolJSON().readDesiredState(input), std::logic_error);
    }

    TEST_F(TestMessagingProtocolJSON, readDesiredState_missingDomain_throwsRuntimeError)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "id",
                "timestamp": 123456789,
                "payload": {
                    "domains": [
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(input));
        EXPECT_THROW(ProtocolJSON().readDesiredState(input), std::runtime_error);
    }

    TEST_F(TestMessagingProtocolJSON, readDesiredState_missingComponent_throwsRuntimeError)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "id",
                "timestamp": 123456789,
                "payload": {
                    "domains": [
                        {
                            "id": "self-update",
                            "components": [
                            ]
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(input));
        EXPECT_THROW(ProtocolJSON().readDesiredState(input), std::runtime_error);
    }

    TEST_F(TestMessagingProtocolJSON, readDesiredState_missingConfig_throwsRuntimeError)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "id",
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
                                    ]
                                }
                            ]
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(input));
        EXPECT_THROW(ProtocolJSON().readDesiredState(input), std::runtime_error);
    }

    TEST_F(TestMessagingProtocolJSON, readDesiredState_versionIsEmpty_throwsRuntimeError)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "id",
                "timestamp": 123456789,
                "payload": {
                    "domains": [
                        {
                            "id": "self-update",
                            "components": [
                                {
                                    "id": "os-image",
                                    "version": "",
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

        EXPECT_NO_THROW(validateJsonSyntax(input));
        EXPECT_THROW(ProtocolJSON().readDesiredState(input), std::runtime_error);
    }

    TEST_F(TestMessagingProtocolJSON, readDesiredState_bundleDownloadUrlIsEmpty_throwsRuntimeError)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "id",
                "timestamp": 123456789,
                "payload": {
                    "domains": [
                        {
                            "id": "self-update",
                            "components": [
                                {
                                    "id": "os-image",
                                    "version": "1.0",
                                    "config": [
                                        {
                                            "key": "image",
                                            "value": ""
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

        EXPECT_NO_THROW(validateJsonSyntax(input));
        EXPECT_THROW(ProtocolJSON().readDesiredState(input), std::runtime_error);
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

        EXPECT_NO_THROW(validateJsonSyntax(input));
        const sua::DesiredState s = ProtocolJSON().readCurrentStateRequest(input);

        EXPECT_EQ(s.activityId, "random-uuid-as-string");
    }

    TEST_F(TestMessagingProtocolJSON, readCurrentStateRequest_activityIdMissing_throwsLogicError)
    {
        // clang-format off
        const std::string input = R"(
            {
                "timestamp": 123456789
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(input));
        EXPECT_THROW(ProtocolJSON().readCurrentStateRequest(input), std::logic_error);
    }

    TEST_F(TestMessagingProtocolJSON, readCurrentStateRequest_activityIdEmpty_throwsLogicError)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "",
                "timestamp": 123456789
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(input));
        EXPECT_THROW(ProtocolJSON().readCurrentStateRequest(input), std::logic_error);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_systemVersionWithoutActivityId)
    {
        SUA_BUILD_NUMBER = "42";

        ctx.desiredState.activityId = "";
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::SystemVersion);

        // clang-format off
        const std::string expected = R"(
            {
                "timestamp": 42,
                "payload": {
                    "softwareNodes": [
                        {
                            "id": "self-update-agent",
                            "version": "build-42",
                            "name": "OTA NG Self Update Agent",
                            "type": "APPLICATION"
                        },
                        {
                            "id": "self-update:os-image",
                            "version": "1.0",
                            "name": "Official Leda device image",
                            "type": "IMAGE"
                        }
                    ],
                    "hardwareNodes": [],
                    "associations": [
                        {
                            "sourceId": "self-update-agent",
                            "targetId": "self-update:os-image"
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_systemVersionWithActivityId)
    {
        SUA_BUILD_NUMBER = "42";

        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::SystemVersion);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "softwareNodes": [
                        {
                            "id": "self-update-agent",
                            "version": "build-42",
                            "name": "OTA NG Self Update Agent",
                            "type": "APPLICATION"
                        },
                        {
                            "id": "self-update:os-image",
                            "version": "1.0",
                            "name": "Official Leda device image",
                            "type": "IMAGE"
                        }
                    ],
                    "hardwareNodes": [],
                    "associations": [
                        {
                            "sourceId": "self-update-agent",
                            "targetId": "self-update:os-image"
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_identifying)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::Identifying);

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

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_identified)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::Identified);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "IDENTIFIED",
                    "message": "Self-update agent is about to perform an OS image update.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "IDENTIFIED",
                            "progress": 0,
                            "message": "Self-update agent is about to perform an OS image update."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_identificationFailed)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::IdentificationFailed, "test");

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "IDENTIFICATION_FAILED",
                    "message": "test",
                    "actions": []
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_skipped)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::Skipped);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "COMPLETED",
                    "message": "Current OS image is equal to the target one from desired state.",
                    "actions": []
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_rejected)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::Rejected);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "UPDATE_FAILURE",
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

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_downloading)
    {
        d.downloadBytesDownloaded    = 1048576;
        d.downloadBytesTotal         = 10485760;
        d.downloadProgressPercentage = 10;

        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::Downloading);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "DOWNLOADING",
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

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_downloaded)
    {
        d.downloadBytesDownloaded    = 1048576;
        d.downloadBytesTotal         = 10485760;
        d.downloadProgressPercentage = 10;

        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::Downloaded);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "DOWNLOAD_SUCCESS",
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

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_downloadFailed)
    {
        d.downloadProgressPercentage = 66;

        ctx.desiredState.actionStatus  = "DOWNLOAD_FAILURE";
        ctx.desiredState.actionMessage = "Download failed: test";

        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::DownloadFailed);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "DOWNLOAD_FAILURE",
                    "message": "Download failed.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "DOWNLOAD_FAILURE",
                            "progress": 66,
                            "message": "Download failed: test"
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_versionChecking)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::VersionChecking);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "UPDATING",
                    "message": "Self-update agent is performing an OS image update.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "UPDATING",
                            "progress": 0,
                            "message": "Checking bundle version and version in desired state request."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_installing)
    {
        d.installProgressPercentage = 42;

        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::Installing);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "UPDATING",
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

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_installed)
    {
        d.installProgressPercentage = 100;

        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::Installed);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "UPDATE_SUCCESS",
                    "message": "Self-update completed, reboot required.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "UPDATING",
                            "progress": 100,
                            "message": "Writing partition completed, reboot required."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_installFailed)
    {
        d.installProgressPercentage = 66;

        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::InstallFailed);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "UPDATE_FAILURE",
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

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_installFailedFallback)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::InstallFailedFallback);

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
                            "progress": 0,
                            "message": "Install in streaming mode failed, trying in download mode."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_currentState)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::CurrentState);

        EXPECT_EQ_MULTILINE(result, "");
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_cleaned)
    {
        ctx.desiredState.actionStatus  = "STATUS";
        ctx.desiredState.actionMessage = "MESSAGE";

        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::Cleaned);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "CLEANUP_SUCCESS",
                    "message": "Self-update agent has cleaned up after itself.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "STATUS",
                            "progress": 0,
                            "message": "MESSAGE"
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_activating)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::Activating);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "ACTIVATING",
                    "message": "Self-update agent is performing an OS image activation.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "UPDATING",
                            "progress": 0,
                            "message": "Self-update agent is performing an OS image activation."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_activated)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::Activated);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "ACTIVATION_SUCCESS",
                    "message": "Self-update agent has activated the new OS image.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "UPDATED",
                            "progress": 0,
                            "message": "Self-update agent has activated the new OS image."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, createMessage_activationFailed)
    {
        const std::string result = ProtocolJSON().createMessage(ctx, sua::MqttMessage::ActivationFailed);

        // clang-format off
        const std::string expected = R"(
            {
                "activityId": "id",
                "timestamp": 42,
                "payload": {
                    "status": "ACTIVATION_FAILURE",
                    "message": "Self-update agent has failed to activate the new OS image.",
                    "actions": [
                        {
                            "component": {
                                "id": "self-update:os-image",
                                "version": "1.0"
                            },
                            "status": "UPDATE_FAILURE",
                            "progress": 0,
                            "message": "Self-update agent has failed to activate the new OS image."
                        }
                    ]
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(expected));
        EXPECT_NO_THROW(validateJsonSyntax(result));
        EXPECT_EQ_MULTILINE(result, expected);
    }

    TEST_F(TestMessagingProtocolJSON, readCommand_DOWNLOAD)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "random-uuid-as-string",
                "timestamp": 123456789,
                "payload": {
                    "baseline": "BASELINE NAME",
                    "command": "DOWNLOAD"
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(input));
        const auto c = ProtocolJSON().readCommand(input);

        EXPECT_EQ(c.activityId, "random-uuid-as-string");
        EXPECT_EQ(c.event, sua::FotaEvent::DownloadStart);
    }

    TEST_F(TestMessagingProtocolJSON, readCommand_UPDATE)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "random-uuid-as-string",
                "timestamp": 123456789,
                "payload": {
                    "baseline": "BASELINE NAME",
                    "command": "UPDATE"
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(input));
        const auto c = ProtocolJSON().readCommand(input);

        EXPECT_EQ(c.activityId, "random-uuid-as-string");
        EXPECT_EQ(c.event, sua::FotaEvent::InstallStart);
    }

    TEST_F(TestMessagingProtocolJSON, readCommand_CLEANUP)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "random-uuid-as-string",
                "timestamp": 123456789,
                "payload": {
                    "baseline": "BASELINE NAME",
                    "command": "CLEANUP"
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(input));
        const auto c = ProtocolJSON().readCommand(input);

        EXPECT_EQ(c.activityId, "random-uuid-as-string");
        EXPECT_EQ(c.event, sua::FotaEvent::Cleanup);
    }

    TEST_F(TestMessagingProtocolJSON, readCommand_ACTIVATE)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "random-uuid-as-string",
                "timestamp": 123456789,
                "payload": {
                    "baseline": "BASELINE NAME",
                    "command": "ACTIVATE"
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(input));
        const auto c = ProtocolJSON().readCommand(input);

        EXPECT_EQ(c.activityId, "random-uuid-as-string");
        EXPECT_EQ(c.event, sua::FotaEvent::Activate);
    }

    // ROLLBACK
    TEST_F(TestMessagingProtocolJSON, readCommand_ROLLBACK)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "random-uuid-as-string",
                "timestamp": 123456789,
                "payload": {
                    "baseline": "BASELINE NAME",
                    "command": "ROLLBACK"
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(input));
        const auto c = ProtocolJSON().readCommand(input);

        EXPECT_EQ(c.activityId, "random-uuid-as-string");
        EXPECT_EQ(c.event, sua::FotaEvent::Rollback);
    }

    TEST_F(TestMessagingProtocolJSON, readCommand_UNKNOWN_throwsRuntimeError)
    {
        // clang-format off
        const std::string input = R"(
            {
                "activityId": "random-uuid-as-string",
                "timestamp": 123456789,
                "payload": {
                    "baseline": "BASELINE NAME",
                    "command": "UNKNOWN"
                }
            }
        )";
        // clang-format on

        EXPECT_NO_THROW(validateJsonSyntax(input));
        EXPECT_THROW(ProtocolJSON().readCommand(input), std::runtime_error);
    }

}
