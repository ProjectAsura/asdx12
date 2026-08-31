//-----------------------------------------------------------------------------
// File : ChunkAllocatorTest.cpp
// Desc : Chunk Allocator Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxChunkAllocator.h>


namespace {

struct alignas(16) TestValue
{
    int32_t value = 42;
};

} // namespace


TEST(ChunkAllocatorTest, Basic)
{
    asdx::ChunkAllocator allocator;

    uint32_t* a = nullptr;
    int32_t*  b = nullptr;
    float*    c = nullptr;

    do
    {
        a = reinterpret_cast<uint32_t*>(allocator.Alloc(sizeof(uint32_t) * 4));
        b = allocator.New<int32_t>(3);
        c = reinterpret_cast<float*>(allocator.Alloc(sizeof(float) * 2));
    }
    while(allocator.AllocChunk());

    EXPECT_TRUE(a != nullptr);
    EXPECT_TRUE(b != nullptr);
    EXPECT_TRUE(c != nullptr);

    EXPECT_EQ(allocator.GetSize(), sizeof(uint32_t) * 4 + sizeof(int32_t) * 3 + sizeof(float) * 2);
    a[0] = 0u;
    a[1] = 1u;
    a[2] = 2u;
    a[3] = 3u;
    b[0] = 4;
    b[1] = 5;
    b[2] = 6;
    c[0] = 7.5f;
    c[1] = 8.5f;

    EXPECT_EQ(a[0], 0u);
    EXPECT_EQ(a[1], 1u);
    EXPECT_EQ(a[2], 2u);
    EXPECT_EQ(a[3], 3u);
    EXPECT_EQ(b[0], 4);
    EXPECT_EQ(b[1], 5);
    EXPECT_EQ(b[2], 6);
    EXPECT_FLOAT_EQ(c[0], 7.5f);
    EXPECT_FLOAT_EQ(c[1], 8.5f);

    allocator.FreeChunk();
    a = nullptr;
    b = nullptr;
    c = nullptr;

    EXPECT_EQ(allocator.GetSize(), 0u);
}

TEST(ChunkAllocatorTest, Uninitialized)
{
    asdx::ChunkAllocator allocator;

    EXPECT_EQ(allocator.GetSize(), 0u);
    EXPECT_EQ(allocator.New<int32_t>(2), nullptr);
    EXPECT_EQ(allocator.GetSize(), sizeof(int32_t) * 2);

    allocator.FreeChunk();
    EXPECT_EQ(allocator.GetSize(), 0u);
}

TEST(ChunkAllocatorTest, ChunkLifecycle)
{
    asdx::ChunkAllocator allocator;

    auto first = allocator.Alloc(3, 4);
    auto second = allocator.Alloc(5, 8);

    EXPECT_EQ(first, nullptr);
    EXPECT_EQ(second, nullptr);
    EXPECT_EQ(allocator.GetSize(), 13u);

    EXPECT_TRUE(allocator.AllocChunk());
    EXPECT_FALSE(allocator.AllocChunk());

    first = allocator.Alloc(3, 4);
    second = allocator.Alloc(5, 8);
    EXPECT_NE(first, nullptr);
    EXPECT_NE(second, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(first) % 4u, 0u);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(second) % 8u, 0u);

    allocator.FreeChunk();
    EXPECT_EQ(allocator.GetSize(), 0u);

    EXPECT_EQ(allocator.Alloc(8), nullptr);
    EXPECT_EQ(allocator.GetSize(), 8u);
    EXPECT_TRUE(allocator.AllocChunk());
    EXPECT_NE(allocator.Alloc(8), nullptr);

    allocator.FreeChunk();
}

TEST(ChunkAllocatorTest, NewConstructsArray)
{
    asdx::ChunkAllocator allocator;

    EXPECT_EQ(allocator.New<TestValue>(2), nullptr);
    EXPECT_EQ(allocator.GetSize(), sizeof(TestValue) * 2);
    EXPECT_TRUE(allocator.AllocChunk());

    auto values = allocator.New<TestValue>(2);
    ASSERT_NE(values, nullptr);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(values) % alignof(TestValue), 0u);
    EXPECT_EQ(values[0].value, 42);
    EXPECT_EQ(values[1].value, 42);

    values[0].value = 10;
    values[1].value = 20;
    EXPECT_EQ(values[0].value, 10);
    EXPECT_EQ(values[1].value, 20);

    allocator.FreeChunk();
}
