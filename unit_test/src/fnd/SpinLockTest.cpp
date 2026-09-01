//-----------------------------------------------------------------------------
// File : SpinLockTest.cpp
// Desc : Spin Lock Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxSpinLock.h>
#include <atomic>
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

TEST(SpinLockTest, RwSpinLock)
{
    asdx::RwSpinLock<int> lock;

    EXPECT_TRUE(lock.try_read_lock());
    EXPECT_TRUE(lock.try_read_lock());
    EXPECT_FALSE(lock.try_write_lock());

    lock.read_unlock();
    lock.read_unlock();

    EXPECT_TRUE(lock.try_write_lock());
    EXPECT_FALSE(lock.try_read_lock());
    EXPECT_FALSE(lock.try_write_lock());
    lock.write_unlock();

    EXPECT_TRUE(lock.try_read_lock());
    lock.read_unlock();
}

TEST(SpinLockTest, RwSpinLockAllowsConcurrentReaders)
{
    asdx::RwSpinLock<int> lock;
    std::atomic<int> readers = 0;
    std::atomic<bool> start = false;
    std::thread threads[2];

    for (auto& thread : threads)
    {
        thread = std::thread([&]() {
            lock.read_lock();
            ++readers;

            while (!start.load())
            { std::this_thread::yield(); }

            lock.read_unlock();
        });
    }

    while (readers.load() != 2)
    { std::this_thread::yield(); }

    EXPECT_FALSE(lock.try_write_lock());
    start.store(true);

    for (auto& thread : threads)
    { thread.join(); }

    EXPECT_TRUE(lock.try_write_lock());
    lock.write_unlock();
}

TEST(SpinLockTest, ScopedReadLock)
{
    asdx::RwSpinLock<int> lock;

    {
        asdx::ScopedReadLock<asdx::RwSpinLock<int>> scopedLock(lock);
        EXPECT_TRUE(lock.try_read_lock());
        lock.read_unlock();
        EXPECT_FALSE(lock.try_write_lock());
    }

    EXPECT_TRUE(lock.try_write_lock());
    lock.write_unlock();
}

TEST(SpinLockTest, ScopedWriteLock)
{
    asdx::RwSpinLock<int> lock;

    {
        asdx::ScopedWriteLock<asdx::RwSpinLock<int>> scopedLock(lock);
        EXPECT_FALSE(lock.try_read_lock());
        EXPECT_FALSE(lock.try_write_lock());
    }

    EXPECT_TRUE(lock.try_read_lock());
    lock.read_unlock();
}
