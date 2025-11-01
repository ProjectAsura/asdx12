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