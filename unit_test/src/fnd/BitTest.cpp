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

        for(int i=0; i<16; ++i)
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
    EXPECT_EQ(flags.Mask(0x3), 0x1);
    EXPECT_EQ(flags.FindUnused(), 16);

    flags.Set(0, false);
    EXPECT_TRUE(flags.None());
    EXPECT_EQ(flags.FindUnused(), 16);

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

    EXPECT_EQ(asdx::BitFieldExtractSigned(0x00000005, 0, 3), -3);
    EXPECT_EQ(asdx::BitFieldExtractSigned(0x00000003, 0, 3), 3);
}

TEST(BitTest, BitInterleave)
{
    const uint32_t value = 0x1234;
    EXPECT_EQ(asdx::Compact1By1(asdx::Part1By1(value)), value);

    const uint32_t value3 = 0x2aa;
    EXPECT_EQ(asdx::Compact1By2(asdx::Part1By2(value3)), value3);
}

TEST(BitTest, MortonCode)
{
    const uint32_t x2 = 0x1234;
    const uint32_t y2 = 0x5678;
    uint32_t decodedX2 = 0;
    uint32_t decodedY2 = 0;
    asdx::DecodeMorton2(asdx::EncodeMorton2(x2, y2), decodedX2, decodedY2);
    EXPECT_EQ(decodedX2, x2 & 0xffff);
    EXPECT_EQ(decodedY2, y2 & 0xffff);

    const uint32_t x3 = 0x12;
    const uint32_t y3 = 0x23;
    const uint32_t z3 = 0x34;
    uint32_t decodedX3 = 0;
    uint32_t decodedY3 = 0;
    uint32_t decodedZ3 = 0;
    asdx::DecodeMorton3(asdx::EncodeMorton3(x3, y3, z3), decodedX3, decodedY3, decodedZ3);
    EXPECT_EQ(decodedX3, x3);
    EXPECT_EQ(decodedY3, y3);
    EXPECT_EQ(decodedZ3, z3);
}

TEST(BitTest, CountBit)
{
    EXPECT_EQ(asdx::CountBit(uint8_t(0)), 0);
    EXPECT_EQ(asdx::CountBit(uint8_t(0xff)), 8);
    EXPECT_EQ(asdx::CountBit(uint16_t(0x8001)), 2);
    EXPECT_EQ(asdx::CountBit(uint32_t(0x80000001)), 2);
    EXPECT_EQ(asdx::CountBit(uint64_t(0x8000000000000001ull)), 2);
}

TEST(BitTest, BitCast)
{
    const uint32_t bits = 0x3f800000;
    const float value = asdx::bit_cast<float>(bits);
    EXPECT_EQ(value, 1.0f);
    EXPECT_EQ(asdx::bit_cast<uint32_t>(value), bits);
}