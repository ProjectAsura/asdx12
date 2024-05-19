//-----------------------------------------------------------------------------
// File : BitFlagsTest.cpp
// Desc : BitFlags Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxBitFlags.h>


//============
//  8-Bit.
//============
TEST(BitFlagsTest, CountZero8)
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

TEST(BitFlagsTest, CountOne8)
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

TEST(BitFlagsTest, FindZero8)
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

TEST(BitFlagsTest, FindOne8)
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

TEST(BitFlagsTest, BitFlags8)
{
    asdx::BitFlags8 flags;

    EXPECT_EQ((uint8_t)flags, 0);
    EXPECT_TRUE(flags.None());

    flags.Set(0, true);
    EXPECT_TRUE(flags.Get(0));
    EXPECT_FALSE(flags.Get(1));
    EXPECT_TRUE(flags.Any());

    flags = asdx::BitFlags8(0xff);
    EXPECT_EQ((uint8_t)flags, 0xff);
    EXPECT_TRUE(flags.All());

    flags.Reset();
    EXPECT_EQ((uint8_t)flags, 0);
}

//============
//  16-Bit.
//============
TEST(BitFlagsTest, CountZero16)
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

TEST(BitFlagsTest, CountOne16)
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

TEST(BitFlagsTest, FindZero16)
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

TEST(BitFlagsTest, FindOne16)
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

TEST(BitFlagsTest, BitFlags16)
{
    asdx::BitFlags16 flags;

    EXPECT_EQ((uint16_t)flags, 0);
    EXPECT_TRUE(flags.None());

    flags.Set(0, true);
    EXPECT_TRUE(flags.Get(0));
    EXPECT_FALSE(flags.Get(1));
    EXPECT_TRUE(flags.Any());

    flags = asdx::BitFlags16(0xffff);
    EXPECT_EQ((uint16_t)flags, 0xffff);
    EXPECT_TRUE(flags.All());

    flags.Reset();
    EXPECT_EQ((uint16_t)flags, 0);
}

//============
//  32-Bit.
//============
TEST(BitFlagsTest, CountZero32)
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

TEST(BitFlagsTest, CountOne32)
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

TEST(BitFlagsTest, FindZero32)
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

TEST(BitFlagsTest, FindOne32)
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

TEST(BitFlagsTest, BitFlags32)
{
    asdx::BitFlags32 flags;

    EXPECT_EQ((uint32_t)flags, 0);
    EXPECT_TRUE(flags.None());

    flags.Set(0, true);
    EXPECT_TRUE(flags.Get(0));
    EXPECT_FALSE(flags.Get(1));
    EXPECT_TRUE(flags.Any());

    flags = asdx::BitFlags32(0xffffffff);
    EXPECT_EQ((uint32_t)flags, 0xffffffff);
    EXPECT_TRUE(flags.All());

    flags.Reset();
    EXPECT_EQ((uint32_t)flags, 0);
}

//============
//  64-Bit.
//============
TEST(BitFlagsTest, CountZero64)
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

TEST(BitFlagsTest, CountOne64)
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

TEST(BitFlagsTest, FindZero64)
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

TEST(BitFlagsTest, FindOne64)
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

TEST(BitFlagsTest, BitFlags64)
{
    asdx::BitFlags64 flags;

    EXPECT_EQ((uint64_t)flags, 0);
    EXPECT_TRUE(flags.None());

    flags.Set(0, true);
    EXPECT_TRUE(flags.Get(0));
    EXPECT_FALSE(flags.Get(1));
    EXPECT_TRUE(flags.Any());

    flags = asdx::BitFlags64(0xffffffffffffffff);
    EXPECT_EQ((uint64_t)flags, 0xffffffffffffffff);
    EXPECT_TRUE(flags.All());

    flags.Reset();
    EXPECT_EQ((uint64_t)flags, 0);
}