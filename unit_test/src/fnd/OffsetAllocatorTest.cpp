//-----------------------------------------------------------------------------
// File : OffsetAllocatorTest.cpp
// Desc : Offset Allocator Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxOffsetAllocator.h>
#include <vector>

const auto ALLOC_SIZE = 1024u * 1024u * 256u;


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