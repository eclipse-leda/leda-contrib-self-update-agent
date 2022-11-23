#include "gtest/gtest.h"
#include "Mqtt/MqttMessageDeserializer.h"
#include "PayloadMessages.h"

namespace
{
    TEST(TestMessageDeserializer, deserializeStart)
    {
        const std::string input =
            "apiVersion: 1\n"
            "kind: 2\n"
            "metadata: \n"
            "  name: 3\n"
            "spec: \n"
            "  bundleDownloadUrl: 6\n"
            "  bundleName: 4\n"
            "  bundleTarget: 7\n"
            "  bundleVersion: 5\n";

        sua::MessageStart m;
        sua::MqttMessageDeserializer d;
        d.deserialize(input, m);

        EXPECT_EQ(m.apiVersion, "1");
        EXPECT_EQ(m.kind, "2");
        EXPECT_EQ(m.metadataName, "3");
        EXPECT_EQ(m.bundleDownloadUrl, "6");
        EXPECT_EQ(m.bundleName, "4");
        EXPECT_EQ(m.bundleTarget, "7");
        EXPECT_EQ(m.bundleVersion, "5");
    }

    TEST(TestMessageDeserializer, deserializeState)
    {
        const std::string input =
            "apiVersion: 1\n"
            "kind: 2\n"
            "metadata: \n"
            "  name: 3\n"
            "spec: \n"
            "  bundleDownloadUrl: 6\n"
            "  bundleName: 4\n"
            "  bundleTarget: 7\n"
            "  bundleVersion: 5\n"
            "state: \n"
            "  name: 6\n"
            "  progress: 7\n"
            "  techCode: 8\n"
            "  message: 9\n";

        sua::MessageState m;
        sua::MqttMessageDeserializer d;
        d.deserialize(input, m);

        EXPECT_EQ(m.apiVersion, "1");
        EXPECT_EQ(m.kind, "2");
        EXPECT_EQ(m.metadataName, "3");
        EXPECT_EQ(m.bundleDownloadUrl, "6");
        EXPECT_EQ(m.bundleName, "4");
        EXPECT_EQ(m.bundleTarget, "7");
        EXPECT_EQ(m.bundleVersion, "5");
        EXPECT_EQ(m.stateName, "6");
        EXPECT_EQ(m.stateProgress, 7);
        EXPECT_EQ(m.stateTechCode, 8);
        EXPECT_EQ(m.stateMessage, "9");
    }
}
