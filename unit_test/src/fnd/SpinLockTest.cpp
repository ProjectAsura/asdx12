//-----------------------------------------------------------------------------
// File : SpinLockTest.cpp
// Desc : Spin Lock Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxSpinLock.h>
#include <thread>


TEST(SpinLockTest, Basic)
{
    asdx::SpinLock lock;

    EXPECT_TRUE(lock.try_lock());
    EXPECT_FALSE(lock.try_lock());

    lock.unlock();

    EXPECT_TRUE(lock.try_lock());
    lock.unlock();
}

TEST(SpinLockTest, MutualExclusion)
{
    asdx::SpinLock lock;
    int count = 0;

    constexpr int threadCount = 4;
    constexpr int iterationCount = 1000;
    std::thread threads[threadCount];

    for (auto& thread : threads)
    {
        thread = std::thread([&]() {
            for (auto i = 0; i < iterationCount; ++i)
            {
                lock.lock();
                ++count;
                lock.unlock();
            }
        });
    }

    for (auto& thread : threads)
    { thread.join(); }

    EXPECT_EQ(count, threadCount * iterationCount);
}

TEST(SpinLockTest, ScopedLock)
{
    asdx::SpinLock lock;

    {
        asdx::ScopedLock<asdx::SpinLock> scopedLock(lock);
        EXPECT_FALSE(lock.try_lock());
    }

    EXPECT_TRUE(lock.try_lock());
    lock.unlock();
}

TEST(SpinLockTest, RecursiveSpinLock)
{
    asdx::RecursiveSpinLock lock;

    EXPECT_TRUE(lock.try_lock());
    EXPECT_TRUE(lock.try_lock());

    lock.unlock();
    EXPECT_TRUE(lock.try_lock());

    lock.unlock();
    lock.unlock();
    EXPECT_TRUE(lock.try_lock());
    lock.unlock();
}

