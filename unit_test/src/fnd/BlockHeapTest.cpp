//-----------------------------------------------------------------------------
// File : BlockHeapTest.cpp
// Desc : Block Heap Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxBlockHeap.h>


namespace {

struct TestValue
{
    TestValue(int value, float ratio)
    : value(value)
    , ratio(ratio)
    { /* DO_NOTHING */ }

    int   value;
    float ratio;
};

} // namespace


TEST(BlockHeapTest, Basic)
{
    asdx::BlockHeap heap;

    ASSERT_TRUE(heap.Init(32, 4));
    EXPECT_EQ(heap.GetBlockSize(), 32u);
    EXPECT_EQ(heap.GetCapacity(), 4u);
    EXPECT_EQ(heap.GetUsedCount(), 0u);
    EXPECT_EQ(heap.GetFreeCount(), 4u);

    void* blocks[4] = {};
    for(auto i=0u; i<4; ++i)
    {
        blocks[i] = heap.Alloc();
        ASSERT_NE(blocks[i], nullptr);
        EXPECT_EQ(reinterpret_cast<uintptr_t>(blocks[i]) % alignof(std::max_align_t), 0u);
    }

    EXPECT_EQ(heap.GetUsedCount(), 4u);
    EXPECT_EQ(heap.GetFreeCount(), 0u);
    EXPECT_EQ(heap.Alloc(), nullptr);

    for(auto block : blocks)
    { heap.Free(block); }

    EXPECT_EQ(heap.GetUsedCount(), 0u);
    EXPECT_EQ(heap.GetFreeCount(), 4u);

    heap.Term();
}

TEST(BlockHeapTest, CapacityBoundaries)
{
    const uint32_t capacities[] = { 1u, 31u, 32u, 33u, 64u };

    for(auto capacity : capacities)
    {
        asdx::BlockHeap heap;
        ASSERT_TRUE(heap.Init(16, capacity));

        for(auto i=0u; i<capacity; ++i)
        { ASSERT_NE(heap.Alloc(), nullptr); }

        EXPECT_EQ(heap.Alloc(), nullptr);
        EXPECT_EQ(heap.GetUsedCount(), capacity);
        EXPECT_EQ(heap.GetFreeCount(), 0u);

        heap.Term();
    }
}

TEST(BlockHeapTest, ReuseReleasedBlocks)
{
    asdx::BlockHeap heap;
    ASSERT_TRUE(heap.Init(16, 3));

    auto first  = heap.Alloc();
    auto second = heap.Alloc();
    auto third  = heap.Alloc();
    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);
    ASSERT_NE(third, nullptr);

    heap.Free(second);
    EXPECT_EQ(heap.GetUsedCount(), 2u);
    EXPECT_EQ(heap.GetFreeCount(), 1u);

    auto reused = heap.Alloc();
    EXPECT_EQ(reused, second);
    EXPECT_EQ(heap.GetUsedCount(), 3u);
    EXPECT_EQ(heap.GetFreeCount(), 0u);

    heap.Free(first);
    heap.Free(reused);
    heap.Free(third);
    EXPECT_EQ(heap.GetUsedCount(), 0u);
}

TEST(BlockHeapTest, NewConstructsObject)
{
    asdx::BlockHeap heap;
    ASSERT_TRUE(heap.Init(sizeof(TestValue), 1));

    auto value = heap.New<TestValue>(42, 1.5f);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->value, 42);
    EXPECT_FLOAT_EQ(value->ratio, 1.5f);
    EXPECT_EQ(heap.GetUsedCount(), 1u);

    value->~TestValue();
    heap.Free(value);
    EXPECT_EQ(heap.GetFreeCount(), 1u);
}

TEST(BlockHeapTest, TermAndReinitialize)
{
    asdx::BlockHeap heap;
    ASSERT_TRUE(heap.Init(8, 2));
    ASSERT_NE(heap.Alloc(), nullptr);

    heap.Term();
    EXPECT_EQ(heap.GetBlockSize(), 0u);
    EXPECT_EQ(heap.GetCapacity(), 0u);
    EXPECT_EQ(heap.GetUsedCount(), 0u);
    EXPECT_EQ(heap.GetFreeCount(), 0u);

    ASSERT_TRUE(heap.Init(24, 3));
    EXPECT_EQ(heap.GetBlockSize(), 24u);
    EXPECT_EQ(heap.GetCapacity(), 3u);
    EXPECT_EQ(heap.GetUsedCount(), 0u);
    EXPECT_EQ(heap.GetFreeCount(), 3u);
    heap.Term();
}

TEST(BlockHeapTest, ThreadSafeBlockHeap)
{
    asdx::ThreadSafeBlockHeap heap;
    ASSERT_TRUE(heap.Init(16, 2));

    EXPECT_EQ(heap.GetBlockSize(), 16u);
    EXPECT_EQ(heap.GetCapacity(), 2u);
    auto first = heap.Alloc();
    auto second = heap.New<uint64_t>(123u);
    EXPECT_NE(first, nullptr);
    EXPECT_NE(second, nullptr);
    EXPECT_EQ(*second, 123u);
    EXPECT_EQ(heap.GetUsedCount(), 2u);
    EXPECT_EQ(heap.Alloc(), nullptr);

    heap.Free(first);
    heap.Free(second);
    EXPECT_EQ(heap.GetUsedCount(), 0u);
    EXPECT_EQ(heap.GetFreeCount(), 2u);
    heap.Term();
}
