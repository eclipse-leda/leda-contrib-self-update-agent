#include "gtest/gtest.h"
#include "FSM/StateFactory.h"
#include "FSM/State.h"

namespace {

    class A : public sua::State {
    public:
        A()
            : State("A")
        { }
    };

    class B : public sua::State {
    public:
        B()
            : State("B")
        { }
    };

    TEST(TestStateFactory, createKnownState_returnsState)
    {
        sua::StateFactory f;
        f.addStateT<A>("A");
        f.addStateT<B>("B");

        EXPECT_NE(std::dynamic_pointer_cast<A>(f.createState("A")), nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<B>(f.createState("B")), nullptr);
    }

    TEST(TestStateFactory, createUnknownState_throwsLogicError)
    {
        sua::StateFactory f;
        f.addStateT<A>("A");
        f.addStateT<B>("B");

        EXPECT_THROW(f.createState("C"), std::logic_error);
    }

}
