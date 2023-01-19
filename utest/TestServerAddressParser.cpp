#include "gtest/gtest.h"
#include "Utils/ServerAddressParser.h"

namespace
{

    class TestServerAddressParser : public ::testing::Test {
    public:
        void parse(const std::string & address) {
            sua::ServerAddressParser().parse(address, transport, host, port);
        }

        std::string transport;
        std::string host;
        int         port;
    };

    TEST_F(TestServerAddressParser, parse_missingTransport_throws)
    {
        EXPECT_THROW(parse("://host:42"), std::runtime_error);
        EXPECT_THROW(parse("//host:42"), std::runtime_error);
        EXPECT_THROW(parse("host:42"), std::runtime_error);
    }

    TEST_F(TestServerAddressParser, parse_missingHost_throws)
    {
        EXPECT_THROW(parse("tcp://:42"), std::runtime_error);
        EXPECT_THROW(parse("tcp://42"), std::runtime_error);
    }

    TEST_F(TestServerAddressParser, parse_missingPort_throws)
    {
        EXPECT_THROW(parse("tcp://host"), std::runtime_error);
        EXPECT_THROW(parse("tcp://host:"), std::runtime_error);
    }

    TEST_F(TestServerAddressParser, parse_success)
    {
        parse("tcp://host:42");
        EXPECT_EQ(transport, "tcp");
        EXPECT_EQ(host, "host");
        EXPECT_EQ(port, 42);

        parse("TCP://host:42");
        EXPECT_EQ(transport, "tcp");
        EXPECT_EQ(host, "host");
        EXPECT_EQ(port, 42);
    }

}
