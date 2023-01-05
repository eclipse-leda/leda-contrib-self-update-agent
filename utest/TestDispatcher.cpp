#include "gtest/gtest.h"
#include "Patterns/Dispatcher.h"

namespace
{

    class Subscriber : public sua::DispatcherSubscriber {
    public:
    };

    class TestDispatcher : public ::testing::Test {
    public:
        void dispatch(const std::string & target, const std::string & value) {
            std::map<std::string, std::string> payload;
            payload["key"] = value;
            sua::Dispatcher::instance().dispatch(target, payload);
        }
    };

    TEST_F(TestDispatcher, multipleSubscribersAndSingleTopic_receiveAll)
    {
        std::string s1_payload;
        std::string s2_payload;

        Subscriber s1;
        s1.subscribe("a", [this, &s1_payload] (const std::map<std::string, std::string> & payload) {
            s1_payload = payload.at("key");
        });
        
        Subscriber s2;
        s2.subscribe("a", [this, &s2_payload] (const std::map<std::string, std::string> & payload) {
            s2_payload = payload.at("key");
        });

        dispatch("a", "1");

        EXPECT_EQ(s1_payload, "1");
        EXPECT_EQ(s2_payload, "1");
    }

    TEST_F(TestDispatcher, multipleSubscribersAndMultipleTopics_receiveBasedOnTopic)
    {
        std::string s1_payload;
        std::string s2_payload;

        Subscriber s1;
        s1.subscribe("a", [this, &s1_payload] (const std::map<std::string, std::string> & payload) {
            s1_payload = payload.at("key");
        });
        
        Subscriber s2;
        s2.subscribe("b", [this, &s2_payload] (const std::map<std::string, std::string> & payload) {
            s2_payload = payload.at("key");
        });

        dispatch("a", "1");
        EXPECT_EQ(s1_payload, "1");
        EXPECT_EQ(s2_payload, "");

        s1_payload = "";
        s2_payload = "";

        dispatch("b", "2");
        EXPECT_EQ(s1_payload, "");
        EXPECT_EQ(s2_payload, "2");
    }

    TEST_F(TestDispatcher, unsubscribe)
    {
        std::string s1_payload;
        std::string s2_payload;

        {
            // s1 introduced into scope
            Subscriber s1;
            s1.subscribe("a", [this, &s1_payload] (const std::map<std::string, std::string> & payload) {
                s1_payload = payload.at("key");
            });
    
            // dispatch "a" only to s1 and check payload
            dispatch("a", "1");
            EXPECT_EQ(s1_payload, "1");
            EXPECT_EQ(s2_payload, "");

            // reset
            s1_payload = "";
            s2_payload = "";

            {
                // s2 introduced into scope
                Subscriber s2;
                s2.subscribe("b", [this, &s2_payload] (const std::map<std::string, std::string> & payload) {
                    s2_payload = payload.at("key");
                });

                // reset
                s1_payload = "";
                s2_payload = "";

                // dispatch "b" only to s2 and check payload
                dispatch("b", "2");
                EXPECT_EQ(s1_payload, "");
                EXPECT_EQ(s2_payload, "2");
            }

            // reset
            s1_payload = "";
            s2_payload = "";

            // "b" is not subscribed anymore
            dispatch("b", "2");
            EXPECT_EQ(s1_payload, "");
            EXPECT_EQ(s2_payload, "");

            // reset
            s1_payload = "";
            s2_payload = "";

            // "a" is still subscribed
            dispatch("a", "1");
            EXPECT_EQ(s1_payload, "1");
            EXPECT_EQ(s2_payload, "");
        }

        // reset
        s1_payload = "";
        s2_payload = "";

        // "a" is not subscribed anymore
        dispatch("a", "1");
        EXPECT_EQ(s1_payload, "");
        EXPECT_EQ(s2_payload, "");
    }

}
