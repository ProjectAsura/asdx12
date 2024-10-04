//-----------------------------------------------------------------------------
// File : BitTest.cpp
// Desc : Bit Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxBit.h>


//============
//  8-Bit.
//============
TEST(BitTest, CountZero8)
{
    // CountZeroL
    {
        uint8_t bit = 0;
        EXPECT_EQ(asdx::CountZeroL(bit), 8);

        for(int i=0; i<8; ++i)
        {
            uint8_t mask = 0x1 << i;
            EXPECT_EQ(asdx::CountZeroL(mask), 7 - i);
        }
    }

    // CountZeroR
    {
        uint8_t bit = 0;
        EXPECT_EQ(asdx::CountZeroR(bit), 8);

        for(int i=0; i<8; ++i)
        {
            uint8_t mask = 0x1 << i;
            EXPECT_EQ(asdx::CountZeroR(mask), i);
        }
    }
}

TEST(BitTest, CountOne8)
{
    // CountOneL
    {
        uint8_t bit = 0;
        EXPECT_EQ(asdx::CountOneL(bit), 0);

        for(int i=0; i<8; ++i)
        {
            uint8_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::CountOneL(mask), 7 - i);
        }
    }

    // CountOneR
    {
        uint8_t bit = 0;
        EXPECT_EQ(asdx::CountOneR(bit), 0);

        for(int i=0; i<8; ++i)
        {
            uint8_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::CountOneR(mask), i);
        }
    }
}

TEST(BitTest, FindZero8)
{
    // FindZeroL
    {
        uint8_t bit = 0;
        EXPECT_EQ(asdx::FindZeroL(bit), 8);

        for(int i=0; i<8; ++i)
        {
            uint8_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::FindZeroL(mask), i+1);
        }
    }

    // FindZeroR
    {
        uint8_t bit = 0;
        EXPECT_EQ(asdx::FindZeroR(bit), 1);

        for(int i=0; i<8; ++i)
        {
            uint8_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::FindZeroR(mask), i+1);
        }
    }
}

TEST(BitTest, FindOne8)
{
    // FindOneL
    {
        uint8_t bit = 0;
        EXPECT_EQ(asdx::FindOneL(bit), 0);

        for(int i=0; i<8; ++i)
        {
            uint8_t mask = 0x1 << i;
            EXPECT_EQ(asdx::FindOneL(mask), i+1);
        }
    }

    // FindOneR
    {
        uint8_t bit = 0;
        EXPECT_EQ(asdx::FindOneR(bit), 0);

        for(int i=0; i<8; ++i)
        {
            uint8_t mask = 0x1 << i;
            EXPECT_EQ(asdx::FindOneR(mask), i+1);
        }
    }
}

TEST(BitTest, BitFlag8)
{
    asdx::BitFlag8 flags;

    EXPECT_EQ((uint8_t)flags, 0);
    EXPECT_TRUE(flags.None());

    flags.Set(0, true);
    EXPECT_TRUE(flags.Get(0));
    EXPECT_FALSE(flags.Get(1));
    EXPECT_TRUE(flags.Any());

    flags = asdx::BitFlag8(0xff);
    EXPECT_EQ((uint8_t)flags, 0xff);
    EXPECT_TRUE(flags.All());

    flags.Reset();
    EXPECT_EQ((uint8_t)flags, 0);
}

//============
//  16-Bit.
//============
TEST(BitTest, CountZero16)
{
    // CountZeroL
    {
        uint16_t bit = 0;
        EXPECT_EQ(asdx::CountZeroL(bit), 16);

        for(int i=0; i<16; ++i)
        {
            uint16_t mask = 0x1 << i;
            EXPECT_EQ(asdx::CountZeroL(mask), 15 - i);
        }
    }

    // CountZeroR
    {
        uint16_t bit = 0;
        EXPECT_EQ(asdx::CountZeroR(bit), 16);

        for(int i=0; i<16; ++i)
        {
            uint16_t mask = 0x1 << i;
            EXPECT_EQ(asdx::CountZeroR(mask), i);
        }
    }
}

TEST(BitTest, CountOne16)
{
    // CountOneL
    {
        uint16_t bit = 0;
        EXPECT_EQ(asdx::CountOneL(bit), 0);

        for(int i=0; i<16; ++i)
        {
            uint16_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::CountOneL(mask), 15 - i);
        }
    }

    // CountOneR
    {
        uint16_t bit = 0;
        EXPECT_EQ(asdx::CountOneR(bit), 0);

        for(int i=0; i<16; ++i)
        {
            uint16_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::CountOneR(mask), i);
        }
    }
}

TEST(BitTest, FindZero16)
{
    // FindZeroL
    {
        uint16_t bit = 0;
        EXPECT_EQ(asdx::FindZeroL(bit), 16);

        for(int i=0; i<16; ++i)
        {
            uint16_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::FindZeroL(mask), i+1);
        }
    }

    // FindZeroR
    {
        uint16_t bit = 0;
        EXPECT_EQ(asdx::FindZeroR(bit), 1);

        for(int i=0; i<16; ++i)
        {
            uint16_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::FindZeroR(mask), i+1);
        }
    }
}

TEST(BitTest, FindOne16)
{
    // FindOneL
    {
        uint16_t bit = 0;
        EXPECT_EQ(asdx::FindOneL(bit), 0);

        for(int i=0; i<16; ++i)
        {
            uint16_t mask = 0x1 << i;
            EXPECT_EQ(asdx::FindOneL(mask), i+1);
        }
    }

    // FindOneR
    {
        uint16_t bit = 0;
        EXPECT_EQ(asdx::FindOneR(bit), 0);

        for(int i=0; i<8; ++i)
        {
            uint16_t mask = 0x1 << i;
            EXPECT_EQ(asdx::FindOneR(mask), i+1);
        }
    }
}

TEST(BitTest, BitFlag16)
{
    asdx::BitFlag16 flags;

    EXPECT_EQ((uint16_t)flags, 0);
    EXPECT_TRUE(flags.None());

    flags.Set(0, true);
    EXPECT_TRUE(flags.Get(0));
    EXPECT_FALSE(flags.Get(1));
    EXPECT_TRUE(flags.Any());

    flags = asdx::BitFlag16(0xffff);
    EXPECT_EQ((uint16_t)flags, 0xffff);
    EXPECT_TRUE(flags.All());

    flags.Reset();
    EXPECT_EQ((uint16_t)flags, 0);
}

//============
//  32-Bit.
//============
TEST(BitTest, CountZero32)
{
    // CountZeroL
    {
        uint32_t bit = 0;
        EXPECT_EQ(asdx::CountZeroL(bit), 32);

        for(int i=0; i<32; ++i)
        {
            uint32_t mask = 0x1 << i;
            EXPECT_EQ(asdx::CountZeroL(mask), 31 - i);
        }
    }

    // CountZeroR
    {
        uint32_t bit = 0;
        EXPECT_EQ(asdx::CountZeroR(bit), 32);

        for(int i=0; i<32; ++i)
        {
            uint32_t mask = 0x1 << i;
            EXPECT_EQ(asdx::CountZeroR(mask), i);
        }
    }
}

TEST(BitTest, CountOne32)
{
    // CountOneL
    {
        uint32_t bit = 0;
        EXPECT_EQ(asdx::CountOneL(bit), 0);

        for(int i=0; i<32; ++i)
        {
            uint32_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::CountOneL(mask), 31 - i);
        }
    }

    // CountOneR
    {
        uint32_t bit = 0;
        EXPECT_EQ(asdx::CountOneR(bit), 0);

        for(int i=0; i<32; ++i)
        {
            uint32_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::CountOneR(mask), i);
        }
    }
}

TEST(BitTest, FindZero32)
{
    // FindZeroL
    {
        uint32_t bit = 0;
        EXPECT_EQ(asdx::FindZeroL(bit), 32);

        for(int i=0; i<32; ++i)
        {
            uint32_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::FindZeroL(mask), i+1);
        }
    }

    // FindZeroR
    {
        uint32_t bit = 0;
        EXPECT_EQ(asdx::FindZeroR(bit), 1);

        for(int i=0; i<32; ++i)
        {
            uint32_t mask = 0x1 << i;
            mask = ~mask;
            EXPECT_EQ(asdx::FindZeroR(mask), i+1);
        }
    }
}

TEST(BitTest, FindOne32)
{
    // FindOneL
    {
        uint32_t bit = 0;
        EXPECT_EQ(asdx::FindOneL(bit), 0);

        for(int i=0; i<32; ++i)
        {
            uint32_t mask = 0x1 << i;
            EXPECT_EQ(asdx::FindOneL(mask), i+1);
        }
    }

    // FindOneR
    {
        uint32_t bit = 0;
        EXPECT_EQ(asdx::FindOneR(bit), 0);

        for(int i=0; i<32; ++i)
        {
            uint32_t mask = 0x1 << i;
            EXPECT_EQ(asdx::FindOneR(mask), i+1);
        }
    }
}

TEST(BitTest, BitFlag32)
{
    asdx::BitFlag32 flags;

    EXPECT_EQ((uint32_t)flags, 0);
    EXPECT_TRUE(flags.None());

    flags.Set(0, true);
    EXPECT_TRUE(flags.Get(0));
    EXPECT_FALSE(flags.Get(1));
    EXPECT_TRUE(flags.Any());

    flags = asdx::BitFlag32(0xffffffff);
    EXPECT_EQ((uint32_t)flags, 0xffffffff);
    EXPECT_TRUE(flags.All());

    flags.Reset();
    EXPECT_EQ((uint32_t)flags, 0);
}

//============
//  64-Bit.
//============
TEST(BitTest, CountZero64)
{
    // CountZeroL
    {
        uint64_t bit = 0;
        EXPECT_EQ(asdx::CountZeroL(bit), 64);

        for(int i=0; i<64; ++i)
        {
            uint64_t mask = 0x1ull << i;
            EXPECT_EQ(asdx::CountZeroL(mask), 63 - i);
        }
    }

    // CountZeroR
    {
        uint64_t bit = 0;
        EXPECT_EQ(asdx::CountZeroR(bit), 64);

        for(int i=0; i<64; ++i)
        {
            uint64_t mask = 0x1ull << i;
            EXPECT_EQ(asdx::CountZeroR(mask), i);
        }
    }
}

TEST(BitTest, CountOne64)
{
    // CountOneL
    {
        uint64_t bit = 0;
        EXPECT_EQ(asdx::CountOneL(bit), 0);

        for(int i=0; i<64; ++i)
        {
            uint64_t mask = 0x1ull << i;
            mask = ~mask;
            EXPECT_EQ(asdx::CountOneL(mask), 63 - i);
        }
    }

    // CountOneR
    {
        uint64_t bit = 0;
        EXPECT_EQ(asdx::CountOneR(bit), 0);

        for(int i=0; i<64; ++i)
        {
            uint64_t mask = 0x1ull << i;
            mask = ~mask;
            EXPECT_EQ(asdx::CountOneR(mask), i);
        }
    }
}

TEST(BitTest, FindZero64)
{
    // FindZeroL
    {
        uint64_t bit = 0;
        EXPECT_EQ(asdx::FindZeroL(bit), 64);

        for(int i=0; i<64; ++i)
        {
            uint64_t mask = 0x1ull << i;
            mask = ~mask;
            EXPECT_EQ(asdx::FindZeroL(mask), i+1);
        }
    }

    // FindZeroR
    {
        uint64_t bit = 0;
        EXPECT_EQ(asdx::FindZeroR(bit), 1);

        for(int i=0; i<64; ++i)
        {
            uint64_t mask = 0x1ull << i;
            mask = ~mask;
            EXPECT_EQ(asdx::FindZeroR(mask), i+1);
        }
    }
}

TEST(BitTest, FindOne64)
{
    // FindOneL
    {
        uint64_t bit = 0;
        EXPECT_EQ(asdx::FindOneL(bit), 0);

        for(int i=0; i<64; ++i)
        {
            uint64_t mask = 0x1ull << i;
            EXPECT_EQ(asdx::FindOneL(mask), i+1);
        }
    }

    // FindOneR
    {
        uint64_t bit = 0;
        EXPECT_EQ(asdx::FindOneR(bit), 0);

        for(int i=0; i<64; ++i)
        {
            uint64_t mask = 0x1ull << i;
            EXPECT_EQ(asdx::FindOneR(mask), i+1);
        }
    }
}

TEST(BitTest, BitFlag64)
{
    asdx::BitFlag64 flags;

    EXPECT_EQ((uint64_t)flags, 0);
    EXPECT_TRUE(flags.None());

    flags.Set(0, true);
    EXPECT_TRUE(flags.Get(0));
    EXPECT_FALSE(flags.Get(1));
    EXPECT_TRUE(flags.Any());

    flags = asdx::BitFlag64(0xffffffffffffffff);
    EXPECT_EQ((uint64_t)flags, 0xffffffffffffffff);
    EXPECT_TRUE(flags.All());

    flags.Reset();
    EXPECT_EQ((uint64_t)flags, 0);
}

TEST(BitTest, BitOp)
{
    uint32_t val = 0;
    val = asdx::BitFieldInsert(val, 0x3, 0, 2);
    EXPECT_EQ(val, 0x3);

    val = asdx::BitFieldInsert(val, 0x1, 3, 1);
    EXPECT_EQ(val, 0xb);

    uint32_t ext = 0;
    ext = asdx::BitFieldExtract(val, 3, 1);
    EXPECT_EQ(ext, 0x1);

    ext = asdx::BitFieldExtract(val, 0, 2);
    EXPECT_EQ(ext, 0x3);
}