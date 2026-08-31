//-----------------------------------------------------------------------------
// File : RelativePtrTest.cpp
// Desc : Relative Pointer Test.
// Copyright(c) Project Asura. All right reserved.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <fnd/asdxRelativePtr.h>
#include <cstdio>


struct TestData
{
    uint32_t                    IntCount;
    uint32_t                    FloatCount;
    asdx::RelativePtr<int>      IntValues;
    asdx::RelativePtr<float>    FloatValues;
};

struct RelativePtrObject
{
    int Value;
};

struct RelativePtrOperatorData
{
    asdx::RelativePtr<RelativePtrObject> Object;
    RelativePtrObject                    Target;
};

TEST(RelativePtrTest, Basic)
{
    // テストデータ作成.
    {
        TestData data = {};
        data.IntCount   = 4;
        data.FloatCount = 4;
        data.IntValues  .SetOffset(sizeof(uint32_t) * 2);
        data.FloatValues.SetOffset(sizeof(uint32_t) + sizeof(int) * data.IntCount);

        int intValues[] = { 1, 2, 3, 4 };
        float floatValues[] = { 5.0f, 6.0f, 7.0f, 8.0f };

        FILE* fp = nullptr;
        auto err = fopen_s(&fp, "testdata.dat", "wb");
        EXPECT_TRUE(err == 0);
        if (fp != nullptr)
        {
            fwrite(&data,       sizeof(data),      1, fp);
            fwrite(intValues,   sizeof(int)   * 4, 1, fp);
            fwrite(floatValues, sizeof(float) * 4, 1, fp);
            fclose(fp);
        }
    }

    // テストデータチェック.
    {
        auto size = sizeof(TestData) + sizeof(int) * 4 + sizeof(float) * 4;
        uint8_t* buffer = new uint8_t [size];

        FILE* fp = nullptr;
        auto err = fopen_s(&fp, "testdata.dat", "rb");
        EXPECT_TRUE(err == 0);
        if (fp != nullptr)
        {
            fread(buffer, size, 1, fp);
            fclose(fp);

            TestData* data = reinterpret_cast<TestData*>(buffer);

            EXPECT_EQ(data->IntCount,     4);
            EXPECT_EQ(data->FloatCount,   4);
            EXPECT_EQ(data->IntValues[0], 1);
            EXPECT_EQ(data->IntValues[1], 2);
            EXPECT_EQ(data->IntValues[2], 3);
            EXPECT_EQ(data->IntValues[3], 4);

            EXPECT_FLOAT_EQ(data->FloatValues[0], 5.0f);
            EXPECT_FLOAT_EQ(data->FloatValues[1], 6.0f);
            EXPECT_FLOAT_EQ(data->FloatValues[2], 7.0f);
            EXPECT_FLOAT_EQ(data->FloatValues[3], 8.0f);
        }

        delete[] buffer;
    }

    // テストデータを削除する.
    remove("testdata.dat");
}

TEST(RelativePtrTest, DefaultConstructed)
{
    const asdx::RelativePtr<int> ptr;

    EXPECT_FALSE(ptr);
}

TEST(RelativePtrTest, PointerOperators)
{
    RelativePtrOperatorData data = {};
    data.Target.Value = 42;
    data.Object.SetOffset(static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(&data.Target) - reinterpret_cast<uintptr_t>(&data.Object)));

    EXPECT_TRUE(data.Object);
    EXPECT_EQ(data.Object.operator->(), &data.Target);
    EXPECT_EQ(data.Object->Value, 42);
    EXPECT_EQ((*data.Object).Value, 42);
}
