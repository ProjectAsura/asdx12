//-----------------------------------------------------------------------------
// File : OffsetAllocatorTest.cpp
// Desc : Offset Allocator Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxOffsetAllocator.h>

#include <algorithm>
#include <cstdint>
#include <random>
#include <thread>
#include <utility>
#include <vector>

const auto ALLOC_SIZE = 1024u * 1024u * 256u;

namespace {

void ExpectAllocatorEmpty(asdx::OffsetAllocator& allocator, uint32_t size)
{
    EXPECT_EQ(allocator.GetFreeSize(), size);
    EXPECT_EQ(allocator.GetUsedSize(), 0u);

    auto handle = allocator.Alloc(size);
    ASSERT_TRUE(handle.IsValid());
    EXPECT_EQ(handle.GetOffset(), 0u);
    EXPECT_EQ(handle.GetSize(), size);

    allocator.Free(handle);

    EXPECT_EQ(allocator.GetFreeSize(), size);
    EXPECT_EQ(allocator.GetUsedSize(), 0u);
}

} // namespace


TEST(OffsetAllocatorTest, Basic)
{
    asdx::OffsetAllocator allocator;
    allocator.Init(ALLOC_SIZE);
    auto handle = allocator.Alloc(1337);
    EXPECT_EQ(handle.GetOffset(), 0u);
    EXPECT_EQ(handle.GetSize(), 1337u);
    allocator.Free(handle);
    allocator.Term();
}

TEST(OffsetAllocatorTest, Simple)
{
    asdx::OffsetAllocator allocator;
    allocator.Init(ALLOC_SIZE);

    auto a = allocator.Alloc(0);
    EXPECT_FALSE(a.IsValid());

    auto b = allocator.Alloc(1);
    EXPECT_EQ(b.GetOffset(), 0u);
    EXPECT_EQ(b.GetSize(), 1u);

    auto c = allocator.Alloc(123);
    EXPECT_EQ(c.GetOffset(), 1u);
    EXPECT_EQ(c.GetSize(), 123u);

    auto d = allocator.Alloc(1234);
    EXPECT_EQ(d.GetOffset(), 124u);
    EXPECT_EQ(d.GetSize(), 1234u);

    allocator.Free(a);
    allocator.Free(b);
    allocator.Free(c);
    allocator.Free(d);

    auto validateAll = allocator.Alloc(ALLOC_SIZE);
    EXPECT_EQ(validateAll.GetOffset(), 0u);
    allocator.Free(validateAll);
    allocator.Term();
}

TEST(OffsetAllocatorTest, MergeTrivial)
{
    asdx::OffsetAllocator allocator;
    allocator.Init(ALLOC_SIZE);
    auto a = allocator.Alloc(1337);
    EXPECT_EQ(a.GetOffset(), 0u);
    EXPECT_EQ(a.GetSize(), 1337u);
    allocator.Free(a);

    auto b = allocator.Alloc(1337);
    EXPECT_EQ(b.GetOffset(), 0u);
    EXPECT_EQ(b.GetSize(), 1337u);
    allocator.Free(b);

    auto validateAll = allocator.Alloc(ALLOC_SIZE);
    EXPECT_EQ(validateAll.GetOffset(), 0u);
    EXPECT_EQ(validateAll.GetSize(), ALLOC_SIZE);
    allocator.Free(validateAll);
    allocator.Term();
}

TEST(OffsetAllocatorTest, ReuseTrivial)
{
    asdx::OffsetAllocator allocator;
    allocator.Init(ALLOC_SIZE);

    auto a = allocator.Alloc(1024);
    EXPECT_EQ(a.GetOffset(), 0u);
    EXPECT_EQ(a.GetSize(), 1024);

    auto b = allocator.Alloc(3456);
    EXPECT_EQ(b.GetOffset(), 1024u);
    EXPECT_EQ(b.GetSize(), 3456u);

    allocator.Free(a);

    auto c = allocator.Alloc(1024);
    EXPECT_EQ(c.GetOffset(), 0u);
    EXPECT_EQ(c.GetSize(), 1024u);

    allocator.Free(c);
    allocator.Free(b);

    auto validateAll = allocator.Alloc(ALLOC_SIZE);
    EXPECT_EQ(validateAll.GetOffset(), 0u);
    EXPECT_EQ(validateAll.GetSize(), ALLOC_SIZE);
    allocator.Free(validateAll);

    allocator.Term();
}

TEST(OffsetAllocatorTest, ReuseComplex)
{
    asdx::OffsetAllocator allocator;
    allocator.Init(ALLOC_SIZE);

    auto a = allocator.Alloc(1024);
    EXPECT_EQ(a.GetOffset(), 0u);

    auto b = allocator.Alloc(3456);
    EXPECT_EQ(b.GetOffset(), 1024u);

    allocator.Free(a);

    auto c = allocator.Alloc(2345);
    EXPECT_EQ(c.GetOffset(), 1024 + 3456);

    auto d = allocator.Alloc(456);
    EXPECT_EQ(d.GetOffset(), 0u);

    auto e = allocator.Alloc(512);
    EXPECT_EQ(e.GetOffset(), 456u);

    EXPECT_EQ(allocator.GetFreeSize(), uint32_t(ALLOC_SIZE - 3456 - 2345 - 456 - 512));

    allocator.Free(c);
    allocator.Free(d);
    allocator.Free(b);
    allocator.Free(e);

    auto validateAll = allocator.Alloc(ALLOC_SIZE);
    EXPECT_EQ(validateAll.GetOffset(), 0u);
    allocator.Free(validateAll);

    allocator.Term();
}

TEST(OffsetAllocator, ZeroFragmentation)
{
    asdx::OffsetAllocator allocator;
    allocator.Init(ALLOC_SIZE);

    asdx::OffsetHandle allocations[256];
    for (uint32_t i = 0; i < 256; i++)
    {
        allocations[i] = allocator.Alloc(1024 * 1024);
        EXPECT_EQ(allocations[i].GetOffset(), i * 1024u * 1024u);
    }

    EXPECT_EQ(allocator.GetFreeSize(), 0u);

    allocator.Free(allocations[243]);
    allocator.Free(allocations[5]);
    allocator.Free(allocations[123]);
    allocator.Free(allocations[95]);

    allocator.Free(allocations[151]);
    allocator.Free(allocations[152]);
    allocator.Free(allocations[153]);
    allocator.Free(allocations[154]);

    allocations[243] = allocator.Alloc(1024 * 1024);
    allocations[5]   = allocator.Alloc(1024 * 1024);
    allocations[123] = allocator.Alloc(1024 * 1024);
    allocations[95]  = allocator.Alloc(1024 * 1024);
    allocations[151] = allocator.Alloc(1024 * 1024 * 4); // 4x larger
    EXPECT_TRUE(allocations[243].IsValid());
    EXPECT_TRUE(allocations[5]  .IsValid());
    EXPECT_TRUE(allocations[123].IsValid());
    EXPECT_TRUE(allocations[95] .IsValid());
    EXPECT_TRUE(allocations[151].IsValid());

    for (uint32_t i = 0; i < 256; i++)
    {
        if (i < 152 || i > 154)
            allocator.Free(allocations[i]);
    }

    EXPECT_EQ(allocator.GetFreeSize(), ALLOC_SIZE);

    auto validateAll = allocator.Alloc(ALLOC_SIZE);
    EXPECT_EQ(validateAll.GetOffset(), 0u);
    EXPECT_EQ(validateAll.GetSize(), ALLOC_SIZE);
    allocator.Free(validateAll);

    allocator.Term();
}

TEST(OffsetAllocatorTest, BoundaryCheck)
{
    asdx::OffsetAllocator allocator;
    allocator.Init(ALLOC_SIZE);

    EXPECT_EQ(allocator.GetFreeSize(), ALLOC_SIZE);
    EXPECT_EQ(allocator.GetUsedSize(), 0u);

    auto allocation = allocator.Alloc(ALLOC_SIZE);
    EXPECT_EQ(allocation.GetOffset(), 0u);

    EXPECT_EQ(allocator.GetFreeSize(), 0u);
    EXPECT_EQ(allocator.GetUsedSize(), ALLOC_SIZE);

    allocator.Free(allocation);
    EXPECT_EQ(allocator.GetFreeSize(), ALLOC_SIZE);
    EXPECT_EQ(allocator.GetUsedSize(), 0);

    constexpr uint32_t maxAllocs = 128  * 1024;
    constexpr uint32_t unitSize  = 1024 * 1024 * 256 / maxAllocs;

    std::vector<asdx::OffsetHandle> handles;
    handles.resize(maxAllocs);

    for(uint32_t i=0; i<maxAllocs; ++i)
    {
        handles[i] = allocator.Alloc(unitSize);
        auto& h = handles[i];
        EXPECT_EQ(h.GetOffset(), i * unitSize);
        EXPECT_EQ(h.GetSize(), unitSize);
        EXPECT_TRUE(h.IsValid());
    }

    EXPECT_EQ(allocator.GetFreeSize(), 0);
    EXPECT_EQ(allocator.GetUsedSize(), ALLOC_SIZE);

    for(size_t i=0; i<maxAllocs; ++i)
    {
        allocator.Free(handles[i]);
    }

    EXPECT_EQ(allocator.GetFreeSize(), ALLOC_SIZE);
    EXPECT_EQ(allocator.GetUsedSize(), 0u);

    allocation = allocator.Alloc(4);
    EXPECT_EQ(allocation.GetOffset(), 0u);
    EXPECT_EQ(allocation.GetSize(), 4u);

    allocator.Free(allocation);

    handles.clear();
    handles.shrink_to_fit();

    allocator.Term();
}

TEST(OffsetAllocatorTest, FreeListExhaustionAndReuse)
{
    constexpr uint32_t allocSize  = 64;
    constexpr uint32_t blockSize  = 8;
    constexpr uint32_t blockCount = allocSize / blockSize;

    asdx::OffsetAllocator allocator;

    // Node数を意図的に小さくしてフリーNodeを使い切る.
    allocator.Init(allocSize, blockCount);

    std::vector<asdx::OffsetHandle> handles;
    handles.reserve(blockCount);

    for (uint32_t i = 0; i < blockCount; ++i)
    {
        auto handle = allocator.Alloc(blockSize);
        ASSERT_TRUE(handle.IsValid());
        EXPECT_EQ(handle.GetOffset(), i * blockSize);
        handles.push_back(handle);
    }

    EXPECT_EQ(allocator.GetFreeSize(), 0u);

    for (auto& handle : handles)
    {
        allocator.Free(handle);
        EXPECT_FALSE(handle.IsValid());
    }

    EXPECT_EQ(allocator.GetFreeSize(), allocSize);

    // 全Nodeを解放した後、同じNodeを再利用できることを確認.
    handles.clear();

    for (uint32_t i = 0; i < blockCount; ++i)
    {
        auto handle = allocator.Alloc(blockSize);
        ASSERT_TRUE(handle.IsValid());
        EXPECT_EQ(handle.GetOffset(), i * blockSize);
        handles.push_back(handle);
    }

    for (auto& handle : handles)
    {
        allocator.Free(handle);
    }

    EXPECT_EQ(allocator.GetFreeSize(), allocSize);
}

TEST(OffsetAllocatorTest, FreeOrderReverse)
{
    constexpr uint32_t allocSize  = 1024;
    constexpr uint32_t blockSize  = 32;
    constexpr uint32_t blockCount = allocSize / blockSize;

    asdx::OffsetAllocator allocator;
    allocator.Init(allocSize, blockCount);

    std::vector<asdx::OffsetHandle> handles;
    handles.reserve(blockCount);

    for (uint32_t i = 0; i < blockCount; ++i)
    {
        auto handle = allocator.Alloc(blockSize);
        ASSERT_TRUE(handle.IsValid());
        handles.push_back(handle);
    }

    for (auto itr = handles.rbegin(); itr != handles.rend(); ++itr)
    {
        allocator.Free(*itr);
    }

    ExpectAllocatorEmpty(allocator, allocSize);
}

TEST(OffsetAllocatorTest, FreeOrderRandom)
{
    constexpr uint32_t allocSize  = 4096;
    constexpr uint32_t blockSize  = 64;
    constexpr uint32_t blockCount = allocSize / blockSize;

    asdx::OffsetAllocator allocator;
    allocator.Init(allocSize, blockCount);

    std::vector<asdx::OffsetHandle> handles;
    handles.reserve(blockCount);

    for (uint32_t i = 0; i < blockCount; ++i)
    {
        auto handle = allocator.Alloc(blockSize);
        ASSERT_TRUE(handle.IsValid());
        handles.push_back(handle);
    }

    std::vector<uint32_t> order(blockCount);
    for (uint32_t i = 0; i < blockCount; ++i)
    {
        order[i] = i;
    }

    std::mt19937 random(0x12345678u);
    std::shuffle(order.begin(), order.end(), random);

    for (auto index : order)
    {
        allocator.Free(handles[index]);
    }

    ExpectAllocatorEmpty(allocator, allocSize);
}

TEST(OffsetAllocatorTest, DoubleFree)
{
    asdx::OffsetAllocator allocator;
    allocator.Init(1024);

    auto handle = allocator.Alloc(128);
    ASSERT_TRUE(handle.IsValid());

    allocator.Free(handle);

    EXPECT_FALSE(handle.IsValid());
    EXPECT_EQ(allocator.GetFreeSize(), 1024u);

    // 2回目は何もせず、安全に戻ること.
    allocator.Free(handle);

    EXPECT_FALSE(handle.IsValid());
    EXPECT_EQ(allocator.GetFreeSize(), 1024u);

    auto reused = allocator.Alloc(1024);
    ASSERT_TRUE(reused.IsValid());
    allocator.Free(reused);
}

TEST(OffsetAllocatorTest, InvalidHandle)
{
    asdx::OffsetAllocator allocator;
    allocator.Init(1024);

    asdx::OffsetHandle handle;

    EXPECT_FALSE(handle.IsValid());

    allocator.Free(handle);

    EXPECT_EQ(allocator.GetFreeSize(), 1024u);
    EXPECT_EQ(allocator.GetUsedSize(), 0u);

    auto zeroSize = allocator.Alloc(0);

    EXPECT_FALSE(zeroSize.IsValid());
    EXPECT_EQ(allocator.GetFreeSize(), 1024u);
}

TEST(OffsetAllocatorTest, ResetAfterAlloc)
{
    constexpr uint32_t allocSize = 4096;

    asdx::OffsetAllocator allocator;
    allocator.Init(allocSize);

    auto first = allocator.Alloc(512);
    ASSERT_TRUE(first.IsValid());

    EXPECT_EQ(allocator.GetUsedSize(), 512u);
    EXPECT_EQ(allocator.GetFreeSize(), allocSize - 512u);

    allocator.Reset();

    EXPECT_EQ(allocator.GetUsedSize(), 0u);
    EXPECT_EQ(allocator.GetFreeSize(), allocSize);

    auto second = allocator.Alloc(allocSize);
    ASSERT_TRUE(second.IsValid());
    EXPECT_EQ(second.GetOffset(), 0u);
    EXPECT_EQ(second.GetSize(), allocSize);

    allocator.Free(second);

    EXPECT_EQ(allocator.GetFreeSize(), allocSize);
}

TEST(OffsetAllocatorTest, ReinitializeAfterTerm)
{
    constexpr uint32_t firstSize = 1024;
    constexpr uint32_t secondSize = 4096;

    asdx::OffsetAllocator allocator;

    allocator.Init(firstSize);

    auto first = allocator.Alloc(firstSize);
    ASSERT_TRUE(first.IsValid());
    allocator.Free(first);

    allocator.Term();

    EXPECT_EQ(allocator.GetFreeSize(), 0u);
    EXPECT_EQ(allocator.GetUsedSize(), 0u);

    allocator.Init(secondSize);

    auto second = allocator.Alloc(secondSize);
    ASSERT_TRUE(second.IsValid());
    EXPECT_EQ(second.GetOffset(), 0u);
    EXPECT_EQ(second.GetSize(), secondSize);

    allocator.Free(second);
    allocator.Term();
}

TEST(OffsetAllocatorTest, MoveAllocator)
{
    constexpr uint32_t allocSize = 4096;

    asdx::OffsetAllocator source;
    source.Init(allocSize);

    auto handle = source.Alloc(512);
    ASSERT_TRUE(handle.IsValid());

    asdx::OffsetAllocator destination(std::move(source));

    EXPECT_EQ(destination.GetUsedSize(), 512u);
    EXPECT_EQ(destination.GetFreeSize(), allocSize - 512u);

    destination.Free(handle);

    EXPECT_FALSE(handle.IsValid());
    EXPECT_EQ(destination.GetUsedSize(), 0u);
    EXPECT_EQ(destination.GetFreeSize(), allocSize);

    auto reused = destination.Alloc(allocSize);
    ASSERT_TRUE(reused.IsValid());

    destination.Free(reused);
}

TEST(OffsetAllocatorTest, BinBoundarySizes)
{
    constexpr uint32_t allocSize = 4096;

    const uint32_t sizes[] =
    {
        1, 2, 3, 7, 8, 9,
        15,  16,  17,
        31,  32,  33,
        63,  64,  65,
        127, 128, 129,
        255, 256, 257
    };

    asdx::OffsetAllocator allocator;
    allocator.Init(allocSize);

    std::vector<asdx::OffsetHandle> handles;

    for (auto size : sizes)
    {
        auto handle = allocator.Alloc(size);
        ASSERT_TRUE(handle.IsValid());
        EXPECT_EQ(handle.GetSize(), size);
        handles.push_back(handle);
    }

    for (auto& handle : handles)
    {
        allocator.Free(handle);
    }

    ExpectAllocatorEmpty(allocator, allocSize);
}

TEST(OffsetAllocatorTest, AlignmentRoundsAllocationSize)
{
    asdx::OffsetAllocator allocator;
    allocator.Init(4096);

    auto a = allocator.Alloc(1, 1);
    ASSERT_TRUE(a.IsValid());
    EXPECT_EQ(a.GetSize(), 1u);

    auto b = allocator.Alloc(1, 2);
    ASSERT_TRUE(b.IsValid());
    EXPECT_EQ(b.GetSize(), 2u);

    auto c = allocator.Alloc(3, 4);
    ASSERT_TRUE(c.IsValid());
    EXPECT_EQ(c.GetSize(), 4u);

    auto d = allocator.Alloc(17, 16);
    ASSERT_TRUE(d.IsValid());
    EXPECT_EQ(d.GetSize(), 32u);

    allocator.Free(a);
    allocator.Free(b);
    allocator.Free(c);
    allocator.Free(d);

    EXPECT_EQ(allocator.GetFreeSize(), 4096u);
}

TEST(OffsetAllocatorTest, AllocationTooLargeFails)
{
    constexpr uint32_t allocSize = 1024;

    asdx::OffsetAllocator allocator;
    allocator.Init(allocSize);

    auto tooLarge = allocator.Alloc(allocSize + 1);

    EXPECT_FALSE(tooLarge.IsValid());
    EXPECT_EQ(allocator.GetFreeSize(), allocSize);
    EXPECT_EQ(allocator.GetUsedSize(), 0u);

    auto maximum = allocator.Alloc(allocSize);
    ASSERT_TRUE(maximum.IsValid());

    auto another = allocator.Alloc(1);
    EXPECT_FALSE(another.IsValid());

    allocator.Free(maximum);

    EXPECT_EQ(allocator.GetFreeSize(), allocSize);
}

TEST(OffsetAllocatorTest, RepeatedAllocFree)
{
    constexpr uint32_t allocSize = 64 * 1024;
    constexpr uint32_t iterationCount = 10000;

    asdx::OffsetAllocator allocator;
    allocator.Init(allocSize);

    std::mt19937 random(0x87654321u);
    std::uniform_int_distribution<uint32_t> sizeDistribution(1, 4096);

    for (uint32_t i = 0; i < iterationCount; ++i)
    {
        auto size = sizeDistribution(random);

        auto handle = allocator.Alloc(size);
        ASSERT_TRUE(handle.IsValid());

        EXPECT_EQ(handle.GetSize(), size);

        allocator.Free(handle);
        EXPECT_FALSE(handle.IsValid());
    }

    EXPECT_EQ(allocator.GetFreeSize(), allocSize);
    EXPECT_EQ(allocator.GetUsedSize(), 0u);
}

TEST(OffsetAllocatorTest, ThreadSafeConcurrentAllocFree)
{
    constexpr uint32_t allocSize      = 1024 * 1024;
    constexpr uint32_t threadCount    = 4;
    constexpr uint32_t iterationCount = 5000;

    asdx::ThreadSafeOffsetAllocator allocator;
    allocator.Init(allocSize);

    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    for (uint32_t threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        threads.emplace_back([&allocator, threadIndex, iterationCount]()
        {
            for (uint32_t i = 0; i < iterationCount; ++i)
            {
                const auto size = 16u + ((threadIndex + i) % 256u);

                auto handle = allocator.Alloc(size);
                if (handle.IsValid())
                {
                    allocator.Free(handle);
                }
            }
        });
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    EXPECT_EQ(allocator.GetFreeSize(), allocSize);
    EXPECT_EQ(allocator.GetUsedSize(), 0u);
}
