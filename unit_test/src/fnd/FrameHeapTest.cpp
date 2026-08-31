//-----------------------------------------------------------------------------
// File : FrameHeapTest.cpp
// Desc : Frame Heap Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxFrameHeap.h>


namespace {

struct alignas(16) TestValue
{
    int32_t value = 0;

    explicit TestValue(int32_t value)
    : value(value)
    { /* DO_NOTHING */ }
};

} // namespace


TEST(FrameHeapTest, Uninitialized)
{
    asdx::FrameHeap heap;

    EXPECT_EQ(heap.GetSize(), 0u);
    EXPECT_EQ(heap.GetRestSize(), 0u);
    EXPECT_EQ(heap.Alloc(1), nullptr);
}

TEST(FrameHeapTest, InitializationAndCapacity)
{
    asdx::FrameHeap heap;

    ASSERT_TRUE(heap.Init(32));
    EXPECT_EQ(heap.GetSize(), 32u);
    EXPECT_EQ(heap.GetRestSize(), 32u);

    auto* first = static_cast<uint8_t*>(heap.Alloc(8));
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(heap.GetRestSize(), 24u);

    auto* second = static_cast<uint8_t*>(heap.Alloc(24));
    ASSERT_NE(second, nullptr);
    EXPECT_EQ(heap.GetRestSize(), 0u);
    EXPECT_EQ(heap.Alloc(1), nullptr);

    heap.Term();
    EXPECT_EQ(heap.GetSize(), 0u);
    EXPECT_EQ(heap.GetRestSize(), 0u);
}

TEST(FrameHeapTest, Alignment)
{
    asdx::FrameHeap heap;
    ASSERT_TRUE(heap.Init(64));

    ASSERT_NE(heap.Alloc(3), nullptr);
    auto* value = static_cast<TestValue*>(heap.Alloc(sizeof(TestValue), alignof(TestValue)));

    ASSERT_NE(value, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(value) % alignof(TestValue), 0u);
    value->value = 42;
    EXPECT_EQ(value->value, 42);

    heap.Term();
}

TEST(FrameHeapTest, ResetReusesBuffer)
{
    asdx::FrameHeap heap;
    ASSERT_TRUE(heap.Init(32));

    auto* first = heap.Alloc(8);
    ASSERT_NE(first, nullptr);
    EXPECT_EQ(heap.GetRestSize(), 24u);

    heap.Reset();
    EXPECT_EQ(heap.GetRestSize(), 32u);
    EXPECT_EQ(heap.Alloc(32), first);
    EXPECT_EQ(heap.GetRestSize(), 0u);
}

TEST(FrameHeapTest, NewConstructsValue)
{
    asdx::FrameHeap heap;
    ASSERT_TRUE(heap.Init(sizeof(TestValue)));

    auto* value = heap.New<TestValue>(123);
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->value, 123);
    EXPECT_EQ(heap.GetRestSize(), 0u);

    heap.Term();
}
