//-----------------------------------------------------------------------------
// File : ChunkAllocatorTest.cpp
// Desc : Chunk Allocator Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxChunkAllocator.h>


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