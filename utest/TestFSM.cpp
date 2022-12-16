#include "gtest/gtest.h"
#include "FSM/State.h"
#include "FSM/FSM.h"
#include "FSM/StateFactory.h"
#include "Context.h"

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

    class C : public sua::State {
    public:
        C()
            : State("C")
        { }
    };

    enum Event {
        EventAtoB,
        EventAtoC,
        EventBtoC,
        EventCtoA
    };

    TEST(TestFSM, setupAndTransitions)
    {
        auto factory = std::make_shared<sua::StateFactory>();
        factory->addStateT<A>("A");
        factory->addStateT<B>("B");
        factory->addStateT<C>("C");

        sua::Context ctx;

        sua::FSM fsm(ctx);
        fsm.setTransitions({
            { static_cast<sua::FotaEvent>(EventAtoB), "A", "B" },
            { static_cast<sua::FotaEvent>(EventAtoC), "A", "C" },
            { static_cast<sua::FotaEvent>(EventBtoC), "B", "C" },
            { static_cast<sua::FotaEvent>(EventCtoA), "C", "A" }
        });
        fsm.setFactory(factory);

        fsm.transitTo("A");
        EXPECT_EQ(fsm.activeState(), "A");

        fsm.handleEvent(static_cast<sua::FotaEvent>(EventAtoB));
        EXPECT_EQ(fsm.activeState(), "B");

        fsm.handleEvent(static_cast<sua::FotaEvent>(EventBtoC));
        EXPECT_EQ(fsm.activeState(), "C");

        fsm.handleEvent(static_cast<sua::FotaEvent>(EventCtoA));
        EXPECT_EQ(fsm.activeState(), "A");

        fsm.handleEvent(static_cast<sua::FotaEvent>(EventAtoC));
        EXPECT_EQ(fsm.activeState(), "C");
    }

}
