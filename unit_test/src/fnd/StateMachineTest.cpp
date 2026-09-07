//-----------------------------------------------------------------------------
// File : StateMachineTest.cpp
// Desc : State Machine Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxStateMachine.h>


namespace {

struct TestListener : public asdx::StateMachine::IStateListener
{
    void OnEnter() override
    { ++EnterCount; }

    void OnLeave() override
    { ++LeaveCount; }

    void OnUpdate(float deltaSec) override
    {
        ++UpdateCount;
        LastDeltaSec = deltaSec;
    }

    const char* GetName() const override
    { return "TestListener"; }

    int EnterCount = 0;
    int LeaveCount = 0;
    int UpdateCount = 0;
    float LastDeltaSec = 0.0f;
};

struct DestructionListener : public TestListener
{
    explicit DestructionListener(bool* destructed)
        : Destructed(destructed)
    {}

    ~DestructionListener() override
    { *Destructed = true; }

    bool* Destructed = nullptr;
};

} // namespace

TEST(StateMachineTest, Basic)
{
    asdx::StateMachine machine;
    EXPECT_EQ(machine.GetState(), asdx::StateMachine::kInvalidState);

    machine.Update(1.0f);

    auto listener1 = new TestListener();
    auto listener2 = new TestListener();

    EXPECT_TRUE(machine.RegisterState(1, listener1));
    EXPECT_TRUE(machine.RegisterState(2, listener2));
    EXPECT_STREQ(listener1->GetName(), "TestListener");

    machine.ChangeState(1);
    EXPECT_EQ(machine.GetState(), 1u);
    EXPECT_EQ(listener1->EnterCount, 1);
    EXPECT_EQ(listener1->LeaveCount, 0);

    machine.Update(0.25f);
    EXPECT_EQ(listener1->UpdateCount, 1);
    EXPECT_FLOAT_EQ(listener1->LastDeltaSec, 0.25f);

    machine.ChangeState(1);
    EXPECT_EQ(listener1->EnterCount, 1);
    EXPECT_EQ(listener1->LeaveCount, 0);

    machine.ChangeState(2);
    EXPECT_EQ(machine.GetState(), 2u);
    EXPECT_EQ(listener1->LeaveCount, 1);
    EXPECT_EQ(listener2->EnterCount, 1);

    machine.ChangeState(99);
    EXPECT_EQ(machine.GetState(), 99u);
    EXPECT_EQ(listener2->LeaveCount, 1);

    machine.Update(0.5f);
    EXPECT_EQ(listener2->UpdateCount, 0);

    machine.ChangeState(1);
    EXPECT_EQ(machine.GetState(), 1u);
    EXPECT_EQ(listener1->EnterCount, 2);

    machine.Update(0.75f);
    EXPECT_EQ(listener1->UpdateCount, 2);
    EXPECT_FLOAT_EQ(listener1->LastDeltaSec, 0.75f);
}

TEST(StateMachineTest, RejectsDuplicateState)
{
    asdx::StateMachine machine;
    auto listener = new TestListener();
    auto duplicate = new TestListener();

    EXPECT_TRUE(machine.RegisterState(1, listener));
    EXPECT_FALSE(machine.RegisterState(1, duplicate));

    delete duplicate;

    machine.ChangeState(1);
    EXPECT_EQ(machine.GetState(), 1u);
    EXPECT_EQ(listener->EnterCount, 1);
}

TEST(StateMachineTest, ReturnsToRegisteredStateAfterUnregisteredState)
{
    asdx::StateMachine machine;
    auto listener = new TestListener();

    EXPECT_TRUE(machine.RegisterState(1, listener));

    machine.ChangeState(1);
    machine.Update(0.25f);
    EXPECT_EQ(listener->UpdateCount, 1);

    machine.ChangeState(99);
    EXPECT_EQ(machine.GetState(), 99u);
    machine.Update(0.5f);
    EXPECT_EQ(listener->UpdateCount, 1);

    machine.ChangeState(1);
    EXPECT_EQ(listener->EnterCount, 2);
    machine.Update(0.75f);
    EXPECT_EQ(listener->UpdateCount, 2);
    EXPECT_FLOAT_EQ(listener->LastDeltaSec, 0.75f);
}

TEST(StateMachineTest, DeletesRegisteredListeners)
{
    bool destructed = false;

    {
        asdx::StateMachine machine;
        EXPECT_TRUE(machine.RegisterState(1, new DestructionListener(&destructed)));
        EXPECT_FALSE(destructed);
    }

    EXPECT_TRUE(destructed);
}
