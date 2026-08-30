//-----------------------------------------------------------------------------
// File : IndexHeapTest.cpp
// Desc : Index Heap Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxIndexHeap.h>


TEST(IndexHeapTest, Basic)
{
    asdx::IndexHeap heap;

    EXPECT_TRUE(heap.Init(128));
    EXPECT_EQ(heap.GetCapacity(), 128u);
    EXPECT_EQ(heap.GetUsedCount(), 0u);
    EXPECT_EQ(heap.GetFreeCount(), 128u);

    for(auto i=0; i<128; ++i)
    {
        auto index = heap.Alloc();
        EXPECT_EQ(index, i);
        EXPECT_TRUE(heap.IsUsed(i));
    }

    EXPECT_EQ(heap.GetFreeCount(), 0u);
    EXPECT_EQ(heap.GetUsedCount(), 128u);

    for(auto i=0; i<128; ++i)
    {
        heap.Free(i);
        EXPECT_FALSE(heap.IsUsed(i));
    }
    EXPECT_EQ(heap.GetFreeCount(), 128u);
    EXPECT_EQ(heap.GetUsedCount(), 0);

    heap.Sort();

    for(auto i=0; i<128; ++i)
    {
        auto index = heap.Alloc();
        EXPECT_EQ(index, i);
        EXPECT_TRUE(heap.IsUsed(i));
    }
    for(auto i=0; i<128; ++i)
    {
        heap.Free(i);
        EXPECT_FALSE(heap.IsUsed(i));
    }
    heap.Sort();

    heap.Term();
}

TEST(IndexHeapTest, Empty)
{
    asdx::IndexHeap heap;

    EXPECT_TRUE(heap.Init(0));
    EXPECT_EQ(heap.GetCapacity(), 0u);
    EXPECT_EQ(heap.GetUsedCount(), 0u);
    EXPECT_EQ(heap.GetFreeCount(), 0u);
    EXPECT_TRUE(heap.IsEmpty());
    EXPECT_TRUE(heap.IsFull());
    EXPECT_EQ(heap.Alloc(), UINT32_MAX);

    heap.Term();
    EXPECT_EQ(heap.GetCapacity(), 0u);
    EXPECT_EQ(heap.GetUsedCount(), 0u);
    EXPECT_EQ(heap.GetFreeCount(), 0u);
}

TEST(IndexHeapTest, CapacityBoundaries)
{
    const uint32_t capacities[] = { 1u, 31u, 32u, 33u, 63u, 64u, 65u };

    for(auto capacity : capacities)
    {
        asdx::IndexHeap heap;
        ASSERT_TRUE(heap.Init(capacity));

        EXPECT_TRUE(heap.IsEmpty());
        EXPECT_FALSE(heap.IsFull());
        for(auto i=0u; i<capacity; ++i)
        {
            EXPECT_EQ(heap.Alloc(), i);
            EXPECT_TRUE(heap.IsUsed(i));
        }
        EXPECT_EQ(heap.Alloc(), UINT32_MAX);
        EXPECT_TRUE(heap.IsFull());
        EXPECT_EQ(heap.GetUsedCount(), capacity);
        EXPECT_EQ(heap.GetFreeCount(), 0u);

        heap.Term();
    }
}

TEST(IndexHeapTest, ReuseReleasedIndices)
{
    asdx::IndexHeap heap;
    ASSERT_TRUE(heap.Init(64));

    for(auto i=0u; i<64; ++i)
    { EXPECT_EQ(heap.Alloc(), i); }

    heap.Free(5);
    heap.Free(1);
    heap.Free(63);
    EXPECT_FALSE(heap.IsFull());
    EXPECT_EQ(heap.GetUsedCount(), 61u);
    EXPECT_EQ(heap.GetFreeCount(), 3u);

    EXPECT_EQ(heap.Alloc(), 1u);
    EXPECT_EQ(heap.Alloc(), 5u);
    EXPECT_EQ(heap.Alloc(), 63u);
    EXPECT_EQ(heap.Alloc(), UINT32_MAX);
    EXPECT_TRUE(heap.IsFull());

    heap.Term();
}

TEST(IndexHeapTest, SortFreeList)
{
    asdx::IndexHeap heap;
    ASSERT_TRUE(heap.Init(96));

    for(auto i=0u; i<96; ++i)
    { EXPECT_EQ(heap.Alloc(), i); }

    for(auto i=96u; i>0; --i)
    { heap.Free(i - 1); }
    EXPECT_TRUE(heap.IsEmpty());

    heap.Sort();
    for(auto i=0u; i<96; ++i)
    {
        EXPECT_EQ(heap.Alloc(), i);
        EXPECT_TRUE(heap.IsUsed(i));
    }
    EXPECT_TRUE(heap.IsFull());

    heap.Term();
}
