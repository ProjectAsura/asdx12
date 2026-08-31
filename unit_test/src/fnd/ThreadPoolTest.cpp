//-----------------------------------------------------------------------------
// File : ThreadPoolTest.cpp
// Desc : Thread Pool Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxThreadPool.h>

#include <atomic>
#include <chrono>
#include <thread>

namespace {

struct TestRunnable : public asdx::IRunnable
{
    explicit TestRunnable(std::atomic<uint32_t>* runCount, uint32_t delay = 0)
        : RunCount(runCount)
        , Delay(delay)
    {}

    void Run() override
    {
        if (Delay != 0)
        { std::this_thread::sleep_for(std::chrono::milliseconds(Delay)); }

        ++(*RunCount);
    }

    std::atomic<uint32_t>* RunCount = nullptr;
    uint32_t Delay = 0;
};

} // namespace

TEST(ThreadPoolTest, CreatePushWaitAndRelease)
{
    asdx::IThreadPool* threadPool = nullptr;
    EXPECT_TRUE(asdx::CreateThreadPool(2, &threadPool));
    EXPECT_TRUE(threadPool != nullptr);

    std::atomic<uint32_t> runCount = 0;
    TestRunnable runnable(&runCount, 10);
    threadPool->Push(&runnable);
    threadPool->Wait();

    EXPECT_EQ(runCount.load(), 1);

    threadPool->Release();
}

TEST(ThreadPoolTest, PushArrayAndReuse)
{
    asdx::IThreadPool* threadPool = nullptr;
    EXPECT_TRUE(asdx::CreateThreadPool(4, &threadPool));
    EXPECT_TRUE(threadPool != nullptr);

    threadPool->Wait();

    std::atomic<uint32_t> runCount = 0;
    TestRunnable runnable0(&runCount, 10);
    TestRunnable runnable1(&runCount, 10);
    TestRunnable runnable2(&runCount, 10);
    TestRunnable runnable3(&runCount, 10);
    TestRunnable runnable4(&runCount, 10);
    TestRunnable runnable5(&runCount, 10);
    TestRunnable runnable6(&runCount, 10);
    TestRunnable runnable7(&runCount, 10);
    asdx::IRunnable* runnablePointers[8] =
    {
        &runnable0, &runnable1, &runnable2, &runnable3,
        &runnable4, &runnable5, &runnable6, &runnable7,
    };

    threadPool->Push(8, runnablePointers);
    threadPool->Wait();
    EXPECT_EQ(runCount.load(), 8);

    TestRunnable singleRunnable(&runCount, 10);
    threadPool->Push(&singleRunnable);
    threadPool->Wait();
    EXPECT_EQ(runCount.load(), 9);

    threadPool->Release();
}

