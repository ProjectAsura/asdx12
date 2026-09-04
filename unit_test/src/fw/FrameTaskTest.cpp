//-----------------------------------------------------------------------------
// File : FrameTaskTest.cpp
// Desc : Frame Task Manager Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fw/asdxFrameTaskManager.h>

namespace {

struct TestTask : public asdx::TaskBase
{
    explicit TestTask(uint32_t* removeCounter = nullptr)
    : removeCounter(removeCounter)
    { /* DO_NOTHING */ }

    void OnCreate() override
    {
        createCount++;
    }

    void OnRemove() override
    {
        removeCount++;
        if (removeCounter != nullptr)
            (*removeCounter)++;
    }

    void OnUpdate(float deltaSec) override
    {
        updateCount++;
        elapsedSec += deltaSec;
    }

    void OnDraw() override
    {
        drawCount++;
    }

    bool IsComplete() const override
    {
        return complete;
    }

    bool IsRemovable() const override
    {
        return removable;
    }

    uint32_t    createCount     = 0;
    uint32_t    removeCount     = 0;
    uint32_t    updateCount     = 0;
    uint32_t    drawCount       = 0;
    float       elapsedSec      = 0.0f;
    bool        complete        = false;
    bool        removable       = false;
    uint32_t*   removeCounter   = nullptr;
};

} // namespace


TEST(FrameTaskManagerTest, InitializationAndCapacity)
{
    asdx::FrameTaskManager manager;

    EXPECT_EQ(manager.GetUsedCount(), 0u);
    EXPECT_EQ(manager.CreateTask<TestTask>(), nullptr);

    ASSERT_TRUE(manager.Init(sizeof(TestTask) * 2));

    auto first = manager.CreateTask<TestTask>();
    auto second = manager.CreateTask<TestTask>();
    ASSERT_NE(first,  nullptr);
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(first->createCount, 1u);
    EXPECT_EQ(second->createCount, 1u);
    EXPECT_EQ(manager.GetUsedCount(), 2u);
    EXPECT_EQ(manager.CreateTask<TestTask>(), nullptr);

    manager.Term();
    EXPECT_EQ(manager.GetUsedCount(), 0u);
}


TEST(FrameTaskManagerTest, TaskLifecycle)
{
    asdx::FrameTaskManager manager;
    ASSERT_TRUE(manager.Init(sizeof(TestTask) * 2));

    auto first  = manager.CreateTask<TestTask>();
    auto second = manager.CreateTask<TestTask>();
    ASSERT_NE(first,  nullptr);
    ASSERT_NE(second, nullptr);

    manager.Update(0.25f);
    manager.Draw();
    EXPECT_EQ(first->updateCount, 1u);
    EXPECT_EQ(first->drawCount, 1u);
    EXPECT_FLOAT_EQ(first->elapsedSec, 0.25f);
    EXPECT_FALSE(manager.IsAllCompleted());

    first ->complete = true;
    second->complete = true;
    EXPECT_TRUE(manager.IsAllCompleted());

    asdx::TaskBase* task = first;
    manager.RemoveTask(task);
    EXPECT_EQ(task, nullptr);
    EXPECT_EQ(manager.GetUsedCount(), 2u);
    EXPECT_EQ(second->removeCount, 0u);

    second->removable = true;
    manager.DeferredRemove();
    EXPECT_EQ(second->removeCount, 1u);
    EXPECT_EQ(manager.GetUsedCount(), 2u);

    manager.Term();
}


TEST(FrameTaskManagerTest, ResetAndTermRemoveTasks)
{
    asdx::FrameTaskManager manager;
    uint32_t removeCounter = 0;
    ASSERT_TRUE(manager.Init(sizeof(TestTask) * 2));

    auto first  = manager.CreateTask<TestTask>(&removeCounter);
    auto second = manager.CreateTask<TestTask>(&removeCounter);
    ASSERT_NE(first,  nullptr);
    ASSERT_NE(second, nullptr);

    manager.Reset();
    EXPECT_EQ(removeCounter, 2u);
    EXPECT_EQ(manager.GetUsedCount(), 0u);

    auto reused = manager.CreateTask<TestTask>(&removeCounter);
    ASSERT_NE(reused, nullptr);
    EXPECT_EQ(reused->createCount, 1u);
    EXPECT_EQ(manager.GetUsedCount(), 1u);

    manager.Term();
    EXPECT_EQ(removeCounter, 3u);
    EXPECT_EQ(manager.GetUsedCount(), 0u);
}

