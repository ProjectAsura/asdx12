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

    EXPECT_TRUE(heap.Init(10));
    EXPECT_TRUE(heap.IsInit());
    EXPECT_EQ(heap.GetUsedCount(), 0);
    EXPECT_EQ(heap.GetFreeCount(), 10);

    auto handle0 = heap.Alloc(1);
    EXPECT_EQ(handle0.GetOffset(), 0);
    EXPECT_EQ(handle0.GetCount(), 1);
    EXPECT_TRUE(handle0.IsValid());
    EXPECT_EQ(heap.GetUsedCount(), 1);
    EXPECT_EQ(heap.GetFreeCount(), 9);

    auto handle1 = heap.Alloc(2);
    EXPECT_EQ(handle1.GetOffset(), 1);
    EXPECT_EQ(handle1.GetCount(), 2);
    EXPECT_TRUE(handle1.IsValid());
    EXPECT_EQ(heap.GetUsedCount(), 3);
    EXPECT_EQ(heap.GetFreeCount(), 7);

    auto handle2 = heap.Alloc(7);
    EXPECT_EQ(handle2.GetOffset(), 3);
    EXPECT_EQ(handle2.GetCount(), 7);
    EXPECT_TRUE(handle1.IsValid());
    EXPECT_EQ(heap.GetUsedCount(), 10);
    EXPECT_EQ(heap.GetFreeCount(), 0);

    auto handle3 = heap.Alloc(1);
    EXPECT_EQ(handle3.GetOffset(), asdx::IndexHandle::INVALID_OFFSET);
    EXPECT_EQ(handle3.GetCount(), 0);
    EXPECT_FALSE(handle3.IsValid());

    heap.Free(handle1);
    EXPECT_EQ(handle1.GetOffset(), asdx::IndexHandle::INVALID_OFFSET);
    EXPECT_EQ(handle1.GetCount(), 0);
    EXPECT_FALSE(handle1.IsValid());
    EXPECT_EQ(heap.GetUsedCount(), 8);
    EXPECT_EQ(heap.GetFreeCount(), 2);

    EXPECT_TRUE(heap.Compact());
    EXPECT_EQ(handle0.GetOffset(), 0);
    EXPECT_EQ(handle0.GetCount(), 1);
    EXPECT_TRUE(handle0.IsValid());

    EXPECT_EQ(handle2.GetOffset(), 1);
    EXPECT_EQ(handle2.GetCount(), 7);
    EXPECT_TRUE(handle2.IsValid());

    EXPECT_EQ(heap.GetUsedCount(), 8);
    EXPECT_EQ(heap.GetFreeCount(), 2);

    heap.Free(handle0);
    EXPECT_EQ(handle0.GetOffset(), asdx::IndexHandle::INVALID_OFFSET);
    EXPECT_EQ(handle0.GetCount(), 0);
    EXPECT_FALSE(handle0.IsValid());
    EXPECT_EQ(heap.GetUsedCount(), 7);
    EXPECT_EQ(heap.GetFreeCount(), 3);

    EXPECT_TRUE(heap.Compact());
    EXPECT_EQ(handle2.GetOffset(), 0);
    EXPECT_EQ(handle2.GetCount(), 7);
    EXPECT_TRUE(handle2.IsValid());
    EXPECT_EQ(heap.GetUsedCount(), 7);
    EXPECT_EQ(heap.GetFreeCount(), 3);

    heap.Free(handle2);
    EXPECT_EQ(handle2.GetOffset(), asdx::IndexHandle::INVALID_OFFSET);
    EXPECT_EQ(handle2.GetCount(), 0);
    EXPECT_FALSE(handle2.IsValid());

    handle0 = heap.Alloc(10);
    EXPECT_EQ(handle0.GetOffset(), 0);
    EXPECT_EQ(handle0.GetCount(), 10);
    EXPECT_TRUE(handle0.IsValid());
    EXPECT_EQ(heap.GetUsedCount(), 10);
    EXPECT_EQ(heap.GetFreeCount(), 0);

    heap.Free(handle0);

    handle1 = heap.Alloc(11);
    EXPECT_EQ(handle1.GetOffset(), asdx::IndexHandle::INVALID_OFFSET);
    EXPECT_EQ(handle1.GetCount(), 0);
    EXPECT_FALSE(handle1.IsValid());

    handle0 = heap.Alloc(1);
    EXPECT_EQ(handle0.GetOffset(), 0);
    EXPECT_EQ(handle0.GetCount(), 1);
    EXPECT_TRUE(handle0.IsValid());

    handle1 = heap.Alloc(1);
    EXPECT_EQ(handle1.GetOffset(), 1);
    EXPECT_EQ(handle1.GetCount(), 1);
    EXPECT_TRUE(handle1.IsValid());

    handle2 = heap.Alloc(2);
    EXPECT_EQ(handle2.GetOffset(), 2);
    EXPECT_EQ(handle2.GetCount(), 2);
    EXPECT_TRUE(handle2.IsValid());

    heap.Free(handle0);
    EXPECT_FALSE(handle0.IsValid());

    handle0 = heap.Alloc(3);
    EXPECT_EQ(handle0.GetOffset(), 4);
    EXPECT_EQ(handle0.GetCount(), 3);
    EXPECT_EQ(heap.GetUsedCount(), 6);
    EXPECT_EQ(heap.GetFreeCount(), 4);

    heap.Free(handle0);
    handle0 = heap.Alloc(1);
    EXPECT_EQ(handle0.GetOffset(), 4);
    EXPECT_EQ(handle0.GetCount(), 1);
    
    heap.Free(handle0);
    heap.Free(handle1);
    heap.Free(handle2);
    
    EXPECT_EQ(heap.GetFreeCount(), 10);
    EXPECT_EQ(heap.GetUsedCount(), 0);

    heap.Term();
    EXPECT_EQ(heap.GetFreeCount(), 0);
    EXPECT_EQ(heap.GetUsedCount(), 0);
    EXPECT_FALSE(heap.IsInit());
}