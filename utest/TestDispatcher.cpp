#include "gtest/gtest.h"
#include "Patterns/Dispatcher.h"

namespace
{

    class Subscriber : public sua::DispatcherSubscriber {
    public:
    };

    TEST(TestDispatcher, multipleSubscribersAndSingleTopic_receiveAll)
    {
        std::string s1_payload;
        std::string s2_payload;

        Subscriber s1;
        s1.subscribe("a", [this, &s1_payload] (const std::string & payload) {
            s1_payload = payload;
        });
        
        Subscriber s2;
        s2.subscribe("a", [this, &s2_payload] (const std::string & payload) {
            s2_payload = payload;
        });

        sua::Dispatcher::instance().dispatch("a", "1");

        EXPECT_EQ(s1_payload, "1");
        EXPECT_EQ(s2_payload, "1");
    }

    TEST(TestDispatcher, multipleSubscribersAndMultipleTopics_receiveBasedOnTopic)
    {
        std::string s1_payload;
        std::string s2_payload;

        Subscriber s1;
        s1.subscribe("a", [this, &s1_payload] (const std::string & payload) {
            s1_payload = payload;
        });
        
        Subscriber s2;
        s2.subscribe("b", [this, &s2_payload] (const std::string & payload) {
            s2_payload = payload;
        });

        sua::Dispatcher::instance().dispatch("a", "1");
        EXPECT_EQ(s1_payload, "1");
        EXPECT_EQ(s2_payload, "");

        s1_payload = "";
        s2_payload = "";

        sua::Dispatcher::instance().dispatch("b", "2");
        EXPECT_EQ(s1_payload, "");
        EXPECT_EQ(s2_payload, "2");
    }

    TEST(TestDispatcher, unsubscribe)
    {
        std::string s1_payload;
        std::string s2_payload;

        {
            // s1 introduced into scope
            Subscriber s1;
            s1.subscribe("a", [this, &s1_payload] (const std::string & payload) {
                s1_payload = payload;
            });
    
            // dispatch "a" only to s1 and check payload
            sua::Dispatcher::instance().dispatch("a", "1");
            EXPECT_EQ(s1_payload, "1");
            EXPECT_EQ(s2_payload, "");

            // reset
            s1_payload = "";
            s2_payload = "";

            {
                // s2 introduced into scope
                Subscriber s2;
                s2.subscribe("b", [this, &s2_payload] (const std::string & payload) {
                    s2_payload = payload;
                });

                // reset
                s1_payload = "";
                s2_payload = "";

                // dispatch "b" only to s2 and check payload
                sua::Dispatcher::instance().dispatch("b", "2");
                EXPECT_EQ(s1_payload, "");
                EXPECT_EQ(s2_payload, "2");
            }

            // reset
            s1_payload = "";
            s2_payload = "";

            // "b" is not subscribed anymore
            sua::Dispatcher::instance().dispatch("b", "2");
            EXPECT_EQ(s1_payload, "");
            EXPECT_EQ(s2_payload, "");

            // reset
            s1_payload = "";
            s2_payload = "";

            // "a" is still subscribed
            sua::Dispatcher::instance().dispatch("a", "1");
            EXPECT_EQ(s1_payload, "1");
            EXPECT_EQ(s2_payload, "");
        }

        // reset
        s1_payload = "";
        s2_payload = "";

        // "a" is not subscribed anymore
        sua::Dispatcher::instance().dispatch("a", "1");
        EXPECT_EQ(s1_payload, "");
        EXPECT_EQ(s2_payload, "");
    }

}
