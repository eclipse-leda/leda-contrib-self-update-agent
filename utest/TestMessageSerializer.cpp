#include "gtest/gtest.h"
#include "Mqtt/MqttMessageSerializer.h"
#include "PayloadMessages.h"

namespace
{
    TEST(TestMessageSerializer, serialize)
    {
        sua::MessageState m;
        m.apiVersion        = "1";
        m.kind              = "2";
        m.metadataName      = "3";
        m.bundleName        = "4";
        m.bundleVersion     = "5";
        m.bundleDownloadUrl = "6";
        m.bundleTarget      = "7";
        m.stateName         = "8";
        m.stateProgress     = 42;
        m.stateTechCode     = 24;
        m.stateMessage      = "0";

        sua::MqttMessageSerializer s;
        const std::string result = s.serialize(m);
        const std::string expected =
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
            "  message: 0\n"
            "  name: 8\n"
            "  progress: 42\n"
            "  techCode: 24\n";
        EXPECT_EQ(result, expected);
    }
}
